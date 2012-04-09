
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

#include "lua.h"
#include "connections.h"
#include "luafuncs_net.h"
#include "luastate.h"

struct luanetstream {
    struct connection c;
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
    //Close the connection contained by the luanetstream object:
    connections_Close(&stream->c);
    //free the stream itself:
    free(stream);
    return 0;
}

static struct luaidref* createnetstreamobj(lua_State* l) {
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
        free(server);
        lua_pushstring(l, "The settings table contains an invalid linebuffered setting: boolean expected");
        return lua_error(l);
    }
    lua_pop(l, 1); //pop linebuffered setting again
    //ok, now it's time to get a connection object:
    struct luaidref* idref = createnetstreamobj(l); //FIXME: this can error! and then "server" won't be freed -> possible memory leak
    if (!idref) {
        free(server);
        lua_pushstring(l, "Couldn't allocate net stream container");
        return lua_error(l);
    }
    //attempt to connect:
    connections_Init(&((struct luanetstream*)idref->ref.ptr)->c, server, port, linebuffered); 
    printf("Target is: %s:%d (%d)\n", server, port, linebuffered);
    free(server);
    return 1; //return the netstream object
}

