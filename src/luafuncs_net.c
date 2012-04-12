
/* blitwizard 2d engine - source code file

  Copyright (C) 2011 Jonas Thiem

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "lua.h"
#include "lauxlib.h"
#include "connections.h"
#include "luafuncs_net.h"
#include "luastate.h"
#include "logging.h" 

struct luanetstream {
    struct connection* c;
};

static struct luanetstream* toluanetstream(lua_State* l, int index) {
    char invalid[] = "Not a valid net stream reference";
    if (lua_type(l, index) != LUA_TUSERDATA) {
        lua_pushstring(l, invalid);
        lua_error(l);
        return NULL;
    }
    if (lua_rawlen(l, index) != sizeof(struct luaidref)) {
        lua_pushstring(l, invalid);
        lua_error(l);
        return NULL;
    }
    struct luaidref* idref = (struct luaidref*)lua_touserdata(l, index);
    if (!idref || idref->magic != IDREF_MAGIC || idref->type != IDREF_NETSTREAM) {
        lua_pushstring(l, invalid);
        lua_error(l);
        return NULL;
    }
    struct luanetstream* obj = idref->ref.ptr;
    return obj;
}

static int garbagecollect_netstream(lua_State* l) {
    //Garbage collect a netstream object:
    struct luanetstream* stream = toluanetstream(l, -1);
    if (!stream) {
        //not a netstream object!
        return 0;
    }

    //close and free stream object
    if (stream->c) {
        stream->c->luarefcount--;
        if (stream->c->luarefcount <= 0) {
            //close connection
            void* p = stream->c;
            connections_Close(stream->c);
            free(stream->c);

            //close all the stored callback functions
            char regname[500];
#ifdef WINDOWS
            snprintf(regname, sizeof(regname), "connectcallback%I64u", (uint64_t)p);
#else
            snprintf(regname, sizeof(regname), "connectcallback%llu", (uint64_t)p);
#endif
            regname[sizeof(regname)-1] = 0;
            lua_pushstring(l, regname);
            lua_pushnil(l);
            lua_settable(l, LUA_REGISTRYINDEX);

#ifdef WINDOWS
            snprintf(regname, sizeof(regname), "readcallback%I64u", (uint64_t)p);
#else
            snprintf(regname, sizeof(regname), "readcallback%llu", (uint64_t)p);
#endif
            regname[sizeof(regname)-1] = 0;
            lua_pushstring(l, regname);
            lua_pushnil(l);
            lua_settable(l, LUA_REGISTRYINDEX);

#ifdef WINDOWS
            snprintf(regname, sizeof(regname), "errorcallback%I64u", (uint64_t)p);
#else
            snprintf(regname, sizeof(regname), "errorcallback%llu", (uint64_t)p);
#endif
            regname[sizeof(regname)-1] = 0;
            lua_pushstring(l, regname);
            lua_pushnil(l);
            lua_settable(l, LUA_REGISTRYINDEX);
        }
    }

    //free the stream itself:
    free(stream);
    return 0;
}

static struct luaidref* createnetstreamobj(lua_State* l, struct connection* use_connection) {
    //Create a luaidref userdata struct which points to a luanetstream:
    struct luaidref* ref = lua_newuserdata(l, sizeof(*ref));
    struct luanetstream* obj = malloc(sizeof(*obj));
    if (!obj) {
        lua_pop(l, 1);
        lua_pushstring(l, "Failed to allocate netstream wrap struct");
        lua_error(l);
        return NULL;
    }

    //initialise structs:
    memset(obj, 0, sizeof(*obj));
    memset(ref, 0, sizeof(*ref));
    ref->magic = IDREF_MAGIC;
    ref->type = IDREF_NETSTREAM;
    ref->ref.ptr = obj;

    if (!use_connection) {
        //allocate connection struct:
        obj->c = malloc(sizeof(struct connection));
        if (!obj->c) {
            free(obj);
            lua_pop(l, 1);
            lua_pushstring(l, "Failed to allocate netstream connection struct");
            lua_error(l);
            return NULL;
        }
        obj->c->luarefcount = 1;
    }else{
        //use the given connection
        obj->c = use_connection;
        obj->c->luarefcount++;
    }

    //make sure it gets garbage collected lateron:
    luastate_SetGCCallback(l, -1, (int (*)(void*))&garbagecollect_netstream);
    return ref;
}

int luafuncs_netopen(lua_State* l) {
    //see if we have a proper settings table:
    if (lua_gettop(l) < 1 || lua_type(l, 1) != LUA_TTABLE) {
        lua_pushstring(l, "First argument is not a valid settings table");
        return lua_error(l);
    }
    int haveread = 0;
    int haveerror = 0;

    //check for a proper 'connected' callback:
    if (lua_gettop(l) < 2 || lua_type(l, 2) != LUA_TFUNCTION) {
        lua_pushstring(l, "Second argument is not a valid connect callback");
        return lua_error(l);
    }

    //check if we have a 'read' callback:
    if (lua_gettop(l) >= 3 && lua_type(l, 3) != LUA_TNIL) {
        if (lua_type(l, 3) != LUA_TFUNCTION) {
            lua_pushstring(l, "Third argument not a function, read callback or nil expected");
            return lua_error(l);
        }
        haveread = 1;
    }

    //check for the 'close' callback:
    if (lua_gettop(l) >= 4 && lua_type(l, 4) != LUA_TNIL) {
        if (lua_type(l, 4) != LUA_TFUNCTION) {
            lua_pushstring(l, "Fourth argument not a function, error callback or nil expected");
            return lua_error(l);
        }
        haveerror = 1;
    }

    //check for server name:
    lua_pushstring(l, "server");
    lua_gettable(l, -2);
    const char* p = lua_tostring(l, -1);
    if (!p) {
        lua_pushstring(l, "The settings table doesn't contain a valid target server name ('server' setting)");
        return lua_error(l);
    }
    char* server = strdup(p);
    lua_pop(l, 1); //pop server name string again

    //check for server port:
    lua_pushstring(l, "port");
    lua_gettable(l, -2);
    if (lua_type(l, -1) != LUA_TNUMBER) {
        free(server);
        lua_pushstring(l, "The settings table doesn't contain a valid target port number ('port'setting)");
        return lua_error(l);
    }
    int port = lua_tointeger(l, -1);
    lua_pop(l, 1); //pop port number again
    if (port < 1 || port > 65535) {
        free(server);
        lua_pushstring(l, "The port number exceeds the possible port range");
        return lua_error(l);
    }

    //check if it should be line buffered:
    int linebuffered = 0;
    lua_pushstring(l, "linebuffered");
    lua_gettable(l, -2);
    if (lua_type(l, -1) == LUA_TBOOLEAN) {
        if (lua_toboolean(l, -1)) {linebuffered = 1;}
    }else{
        if (lua_type(l, -1) != LUA_TNIL) {
            free(server);
            lua_pushstring(l, "The settings table contains an invalid linebuffered setting: boolean expected");
            return lua_error(l);
        }
    }
    lua_pop(l, 1); //pop linebuffered setting again

    //check if we want a low delay connection:
    int lowdelay = 0;
    lua_pushstring(l, "lowdelay");
    lua_gettable(l, -2);
    if (lua_type(l, -1) == LUA_TBOOLEAN) {
        if (lua_toboolean(l, -1)) {linebuffered = 1;}
    }else{
        if (lua_type(l, -1) != LUA_TNIL) {
            free(server);
            lua_pushstring(l, "The settings table contains an invalid lowdelay setting: boolean expected");
            return lua_error(l);
        }
    }
    lua_pop(l, 1); //pop linebuffered setting again

    //ok, now it's time to get a connection object:
    struct luaidref* idref = createnetstreamobj(l, NULL); //FIXME: this can error! and then "server" won't be freed -> possible memory leak
    if (!idref) {
        free(server);
        lua_pushstring(l, "Couldn't allocate net stream container");
        return lua_error(l);
    }

    //set the callbacks onto the metatable of our connection object:
    char regname[500];
#ifdef WINDOWS
    snprintf(regname, sizeof(regname), "connectcallback%I64u", (uint64_t)idref->ref.ptr);
#else
    snprintf(regname, sizeof(regname), "connectcallback%llu", (uint64_t)idref->ref.ptr);
#endif
    regname[sizeof(regname)-1] = 0;
    lua_pushstring(l, regname);
    lua_pushvalue(l, 2);
    lua_settable(l, LUA_REGISTRYINDEX);
    if (haveread) {
#ifdef WINDOWS
        snprintf(regname, sizeof(regname), "readcallback%I64u", (uint64_t)idref->ref.ptr);
#else
        snprintf(regname, sizeof(regname), "readcallback%llu", (uint64_t)idref->ref.ptr);
#endif
        regname[sizeof(regname)-1] = 0;
        lua_pushstring(l, regname);
        lua_pushvalue(l, 3);
        lua_settable(l, LUA_REGISTRYINDEX);
    }
    if (haveerror) {
#ifdef WINDOWS
        snprintf(regname, sizeof(regname), "errorcallback%I64u", (uint64_t)idref->ref.ptr);
#else
        snprintf(regname, sizeof(regname), "errorcallback%llu", (uint64_t)idref->ref.ptr);
#endif
        regname[sizeof(regname)-1] = 0;
        lua_pushstring(l, regname);
        lua_pushvalue(l, 4);
        lua_settable(l, LUA_REGISTRYINDEX);
    }
    
    //attempt to connect:
    connections_Init(((struct luanetstream*)idref->ref.ptr)->c, server, port, linebuffered, lowdelay); 
    free(server);
    return 1; //return the netstream object
}

static int connectedevents(struct connection* c) {
    lua_State* l = luastate_GetStatePtr();

    //push error handling function
    lua_pushcfunction(l, internaltracebackfunc());

    //push connect callback function
    char regname[500];
#ifdef WINDOWS
    snprintf(regname, sizeof(regname), "connectcallback%I64u", (uint64_t)c);
#else
    snprintf(regname, sizeof(regname), "connectcallback%llu", (uint64_t)c);
#endif
    regname[sizeof(regname)-1] = 0;
    lua_pushstring(l, regname);
    lua_gettable(l, LUA_REGISTRYINDEX);

    //push reference to the connection:
    createnetstreamobj(l, c);

    //prompt callback:
    int result = lua_pcall(l, 1, 0, -4);
    lua_pop(l, 1); //pop error handling function again
    if (result != 0) {
        printerror("Error: An error occured when calling blitwiz.net.open() connect callback: %s", lua_tostring(l, -1));
        return 0;
    }
    return 1;
}

static int readevents(struct connection* c, char* data, unsigned int datalength) {
    lua_State* l = luastate_GetStatePtr();

    //push error handling function
    lua_pushcfunction(l, internaltracebackfunc());

    //read callback lua function
    char regname[500];
#ifdef WINDOWS
    snprintf(regname, sizeof(regname), "readcallback%I64u", (uint64_t)c);
#else
    snprintf(regname, sizeof(regname), "readcallback%llu", (uint64_t)c);
#endif
    regname[sizeof(regname)-1] = 0;
    lua_pushstring(l, regname);
    lua_gettable(l, LUA_REGISTRYINDEX);

    //check if we actually have a read callback:
    if (lua_type(l, -1) == LUA_TNIL) {
        lua_pop(l, 1);
        return 1;
    }

    //push reference to the connection:
    createnetstreamobj(l, c);

    //compose data parameter:
    luaL_Buffer b;
    luaL_buffinit(l, &b);
    luaL_addlstring(&b, data, datalength);
    luaL_pushresult(&b);
    
    //prompt callback:
    int result = lua_pcall(l, 2, 0, -4);
    lua_pop(l, 1); //pop error handling function again
    if (result != 0) {
        printerror("Error: An error occured when calling blitwiz.net.open() read callback: %s", lua_tostring(l, -1));
        return 0;
    }
    return 1;
}

static int errorevents(struct connection* c, int error) {
    lua_State* l = luastate_GetStatePtr();

    //push error handling function
    lua_pushcfunction(l, internaltracebackfunc());

    //read callback lua function
    char regname[500];
#ifdef WINDOWS
    snprintf(regname, sizeof(regname), "errorcallback%I64u", (uint64_t)c);
#else
    snprintf(regname, sizeof(regname), "errorcallback%llu", (uint64_t)c);
#endif
    regname[sizeof(regname)-1] = 0;
    lua_pushstring(l, regname);
    lua_gettable(l, LUA_REGISTRYINDEX);

    //check if we actually have an error callback:
    if (lua_type(l, -1) == LUA_TNIL) {
        lua_pop(l, 1);
        return 1;
    }

    //push reference to the connection:
    createnetstreamobj(l, c);

    //push error message:
    switch (error) {
        default:
            lua_pushstring(l, "Unknown connection error");
            break;
    }

    //prompt callback:
    int result = lua_pcall(l, 2, 0, -4);
    lua_pop(l, 1); //pop error handling function again
    if (result != 0) {
        printerror("Error: An error occured when calling blitwiz.net.open() error callback: %s", lua_tostring(l, -1));
        return 0;
    }
    return 1;
}

int luafuncs_ProcessNetEvents() {
    return connections_CheckAll(&readevents, &connectedevents, &errorevents);
}

