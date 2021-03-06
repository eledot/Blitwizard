
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

#include "luaheader.h"
#include "luaerror.h"
#include "connections.h"
#include "luafuncs_net.h"
#include "luastate.h"
#include "logging.h"
#include "timefuncs.h"
#include "sockets.h"
#include "listeners.h"

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

static void clearconnectioncallbacks(struct connection* c) {
    lua_State* l = (lua_State*)c->userdata;
    // close all the stored callback functions
    char regname[500];
    snprintf(regname, sizeof(regname), "opencallback%p", c);
    regname[sizeof(regname)-1] = 0;
    lua_pushstring(l, regname);
    lua_pushnil(l);
    lua_settable(l, LUA_REGISTRYINDEX);

    snprintf(regname, sizeof(regname), "readcallback%p", c);
    regname[sizeof(regname)-1] = 0;
    lua_pushstring(l, regname);
    lua_pushnil(l);
    lua_settable(l, LUA_REGISTRYINDEX);

    snprintf(regname, sizeof(regname), "errorcallback%p", c);
    regname[sizeof(regname)-1] = 0;
    lua_pushstring(l, regname);
    lua_pushnil(l);
    lua_settable(l, LUA_REGISTRYINDEX);
}

static int garbagecollect_netstream(lua_State* l) {
    // Garbage collect a netstream object:
    struct luanetstream* stream = toluanetstream(l, -1);
    if (!stream) {
        // not a netstream object!
        return 0;
    }

    // close and free stream object
    if (stream->c) {
        stream->c->luarefcount--;
        if (stream->c->luarefcount <= 0) {
            int donotclose = 0;

            // maybe we would want to keep the connection open
            if (stream->c->error < 0 || (stream->c->closewhensent && stream->c->outbufbytes > 0)) {
                // keep open if we still intend to send data and the connection is formally already closed:
                if (stream->c->closewhensent && stream->c->outbufbytes > 0) {
#ifdef CONNECTIONSDEBUG
                    printinfo("[connections] keeping %d around for send completion", stream->c->socket);
#endif
                    donotclose = 1;
                    // don't close, however continue here and wipe callbacks:
                }else{

                    // keep open and autoclose when no activity:
                    if (stream->c->canautoclose && stream->c->lastreadtime + 20000 > time_GetMilliseconds()) {
#ifdef CONNECTIONSDEBUG
                        printinfo("[connections] keeping %d around for autoclose", stream->c->socket);
#endif
                        stream->c->wantautoclose = 1;
                        return 0;
                    }
                }
            }

            // close connection
            void* p = stream->c;
            if (!donotclose) {
                connections_Close(stream->c);
                free(stream->c);
            }

            // close all the stored callback functions
            char regname[500];
            snprintf(regname, sizeof(regname), "opencallback%p", stream->c);
            regname[sizeof(regname)-1] = 0;
            lua_pushstring(l, regname);
            lua_pushnil(l);
            lua_settable(l, LUA_REGISTRYINDEX);

            snprintf(regname, sizeof(regname), "readcallback%p", stream->c);
            regname[sizeof(regname)-1] = 0;
            lua_pushstring(l, regname);
            lua_pushnil(l);
            lua_settable(l, LUA_REGISTRYINDEX);

            snprintf(regname, sizeof(regname), "errorcallback%p", stream->c);
            regname[sizeof(regname)-1] = 0;
            lua_pushstring(l, regname);
            lua_pushnil(l);
            lua_settable(l, LUA_REGISTRYINDEX);
        }
    }

    // free the stream itself:
    free(stream);
    return 0;
}

static struct luaidref* createnetstreamobj(lua_State* l, struct connection* use_connection) {
    // Create a luaidref userdata struct which points to a luanetstream:
    struct luaidref* ref = lua_newuserdata(l, sizeof(*ref));
    struct luanetstream* obj = malloc(sizeof(*obj));
    if (!obj) {
        lua_pop(l, 1);
        lua_pushstring(l, "Failed to allocate netstream wrap struct");
        lua_error(l);
        return NULL;
    }

    // initialise structs:
    memset(obj, 0, sizeof(*obj));
    memset(ref, 0, sizeof(*ref));
    ref->magic = IDREF_MAGIC;
    ref->type = IDREF_NETSTREAM;
    ref->ref.ptr = obj;

    if (!use_connection) {
        // allocate connection struct:
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
        // use the given connection
        obj->c = use_connection;
        obj->c->luarefcount++;
        obj->c->wantautoclose = 0; // we got a lua reference now, don't autoclose
    }

    // make sure it gets garbage collected lateron:
    luastate_SetGCCallback(l, -1, (int (*)(void*))&garbagecollect_netstream);
    return ref;
}

static void luafuncs_checkcallbackparameters(lua_State* l, int startindex, const char* functionname, int* haveconnect, int* haveread, int* haveerror) {
    *haveconnect = 0;
    *haveread = 0;
    *haveerror = 0;
    // check for a proper 'connected' callback:
    if (lua_gettop(l) >= startindex && lua_type(l, startindex) != LUA_TNIL) {
        if (lua_type(l, startindex) != LUA_TFUNCTION) {
            haveluaerror(l, badargument1, startindex, functionname, "function", lua_strtype(l, startindex));
        }
        *haveconnect = 1;
    }

    // check if we have a 'read' callback:
    if (lua_gettop(l) >= startindex+1 && lua_type(l, startindex+1) != LUA_TNIL) {
        if (lua_type(l, startindex+1) != LUA_TFUNCTION) {
            haveluaerror(l, badargument1, startindex+1, functionname, "function", lua_strtype(l, startindex+1));
        }
        *haveread = 1;
    }

    // check for the 'close' callback:
    if (lua_gettop(l) >= startindex+2 && lua_type(l, startindex+2) != LUA_TNIL) {
        if (lua_type(l, startindex+2) != LUA_TFUNCTION) {
            haveluaerror(l, badargument1, startindex+2, functionname, "function", lua_strtype(l, startindex+2));
        }
        *haveerror = 1;
    }
}

static void luafuncs_setcallbacks(lua_State* l, void* cptr, int stackindex, int haveconnect, int haveread, int haveerror) {
    // set the callbacks onto the metatable of our connection object:
    char regname[500];
    if (haveconnect) {
        snprintf(regname, sizeof(regname), "opencallback%p", cptr);
        regname[sizeof(regname)-1] = 0;
        lua_pushstring(l, regname);
        lua_pushvalue(l, stackindex);
        lua_settable(l, LUA_REGISTRYINDEX);
    }
    if (haveread) {
        snprintf(regname, sizeof(regname), "readcallback%p", cptr);
        regname[sizeof(regname)-1] = 0;
        lua_pushstring(l, regname);
        lua_pushvalue(l, stackindex+1);
        lua_settable(l, LUA_REGISTRYINDEX);
    }
    if (haveerror) {
        snprintf(regname, sizeof(regname), "errorcallback%p", cptr);
        regname[sizeof(regname)-1] = 0;
        lua_pushstring(l, regname);
        lua_pushvalue(l, stackindex+2);
        lua_settable(l, LUA_REGISTRYINDEX);
    }
}

static void luafuncs_parsestreamsettings(lua_State* l, int stackindex, int argnumber, const char* functionname, int erroronempty, int* port, char** servername, int* linebuffered, int* lowdelay) {
    // see if we have a proper settings table:
    if (lua_gettop(l) < stackindex || lua_type(l, stackindex) == LUA_TNIL) {
        if (erroronempty) {
            haveluaerror(l, badargument1, argnumber, functionname, "table", "nil");
        }
    }
    if (lua_type(l, stackindex) != LUA_TTABLE) {
        haveluaerror(l, badargument1, argnumber, functionname, "table", lua_strtype(l, stackindex));
    }

    // check for server port:
    if (port) {
        lua_pushstring(l, "port");
        lua_gettable(l, stackindex);
        if (lua_type(l, -1) != LUA_TNUMBER) {
            haveluaerror(l, badargument2, argnumber, functionname, "the settings table doesn't contain a valid port setting - please note this setting is not optional");
        }
        *port = lua_tointeger(l, -1);
        lua_pop(l, 1); // pop port number again
        if (*port < 1 || *port > 65535) {
            haveluaerror(l, badargument2, argnumber, functionname, "the given port number exceeds the possible port range");
        }
    }
    // check if it should be line buffered:
    lua_pushstring(l, "linebuffered");
    lua_gettable(l, stackindex);
    if (lua_type(l, -1) == LUA_TBOOLEAN) {
        if (lua_toboolean(l, -1)) {
            *linebuffered = 1;
        }else{
            *linebuffered = 0;
        }
    }else{
        if (lua_type(l, -1) != LUA_TNIL) {
            haveluaerror(l, badargument2, argnumber, functionname, "the settings table contains an invalid linebuffered setting: boolean expected");
        }
    }
    lua_pop(l, 1); // pop linebuffered setting again

    // check if we want a low delay connection:
    lua_pushstring(l, "lowdelay");
    lua_gettable(l, stackindex);
    if (lua_type(l, -1) == LUA_TBOOLEAN) {
        if (lua_toboolean(l, -1)) {
            *lowdelay = 1;
        }else{
            *lowdelay = 0;
        }
    }else{
        if (lua_type(l, -1) != LUA_TNIL) {
            haveluaerror(l, badargument2, argnumber, functionname, "the settings table contains an invalid lowdelay setting: boolean expected");
            return;
        }
    }
    lua_pop(l, 1); // pop linebuffered setting again

    // check for server name:
    if (servername) {
        lua_pushstring(l, "server");
        lua_gettable(l, stackindex);
        const char* p = lua_tostring(l, -1);
        if (!p) {
            haveluaerror(l, badargument2, argnumber, functionname, "the settings table doesn't contain a valid target server name");
            return;
        }
        *servername = strdup(p);
        lua_pop(l, 1); // pop server name string again
    }
}

int luafuncs_netopen(lua_State* l) {
    int haveconnect = 0;
    int haveread = 0;
    int haveerror = 0;

    // check for the callback parameters (will throw lua error if faulty):
    luafuncs_checkcallbackparameters(l, 2, "blitwiz.net.open", &haveconnect, &haveread, &haveerror);

    int port;
    char* server;
    int linebuffered = 0;
    int lowdelay = 0;
    luafuncs_parsestreamsettings(l, 1, 1, "blitwiz.net.open", 1, &port, &server, &linebuffered, &lowdelay);

    // ok, now it's time to get a connection object:
    struct luaidref* idref = createnetstreamobj(l, NULL); // FIXME: this can error! and then "server" won't be freed -> possible memory leak
    if (!idref) {
        free(server);
        return haveluaerror(l, "Couldn't allocate net stream container");
    }

    luafuncs_setcallbacks(l, ((struct luanetstream*)idref->ref.ptr)->c, 2, haveconnect, haveread, haveerror);

    // attempt to connect:
    connections_Init(((struct luanetstream*)idref->ref.ptr)->c, server, port, linebuffered, lowdelay, haveread, clearconnectioncallbacks, (void*)l);
    free(server);
    return 1; // return the netstream object
}

int luafuncs_netsend(lua_State* l) {
    struct luanetstream* netstream = toluanetstream(l, 1);
    if (!connections_CheckIfConnected(netstream->c)) {
        lua_pushstring(l, "Cannot send to a closed stream");
        return lua_error(l);
    }
    size_t sendsize;
    const char *send = luaL_checklstring(l, 2, &sendsize);
    if (sendsize > 0) {
        connections_Send(netstream->c, send, sendsize);
    }
    return 0;
}

int luafuncs_netclose(lua_State* l) {
    if (lua_gettop(l) >= 1 && lua_type(l, 1) == LUA_TNUMBER) {
        // it is a server port
        if (!listeners_CloseByPort(lua_tointeger(l, 1))) {
            return haveluaerror(l, "Cannot close server at port %d - was there a server running at this port?", lua_tointeger(l, 1));
        }else{
            // wipe the callback aswell
            char p[512];
            snprintf(p, sizeof(p), "serverlistenercallback%d", (int)lua_tointeger(l, 1));
            lua_pushstring(l, p);
            lua_pushnil(l);
            lua_settable(l, LUA_REGISTRYINDEX);
        }
        return 0;
    }
    struct luanetstream* netstream = toluanetstream(l, 1);
    if (!connections_CheckIfConnected(netstream->c)) {
        lua_pushstring(l, "Cannot close a stream which is already closed");
        return lua_error(l);
    }
    if (netstream->c->outbufbytes <= 0) {
        so_CloseSSLSocket(netstream->c->socket, &netstream->c->sslptr);
        netstream->c->socket = -1;
        netstream->c->error = CONNECTIONERROR_CONNECTIONCLOSED;
#ifdef CONNECTIONSDEBUG
        printinfo("[connections] close() on %d: closed instantly", netstream->c->socket);
#endif
    }else{
        netstream->c->closewhensent = 1;
        netstream->c->error = CONNECTIONERROR_CONNECTIONCLOSED;
#ifdef CONNECTIONSDEBUG
        printinfo("[connections] close() on %d: will close when %d bytes sent", netstream->c->socket, netstream->c->outbufbytes);
#endif
    }
    return 0;
}

int luafuncs_netset(lua_State* l) {
    struct luanetstream* netstream = toluanetstream(l, 1);
    if (!connections_CheckIfConnected(netstream->c)) {
        lua_pushstring(l, "Cannot set callbacks to a closed stream");
        return lua_error(l);
    }

    // check/retrieve callbacks:
    int haveconnect,haveread,haveerror;
    luafuncs_checkcallbackparameters(l, 3, "blitwiz.net.set", &haveconnect, &haveread, &haveerror);

    // obtain settings:
    int linebuffered = -1;
    int lowdelay = -1;
    luafuncs_parsestreamsettings(l, 2, 2, "blitwiz.net.set", 0, NULL, NULL, &linebuffered, &lowdelay);

    // set callbacks:
    luafuncs_setcallbacks(l, netstream->c, 3, haveconnect, haveread, haveerror);

    // change settings:
    if (linebuffered > 0) {
        netstream->c->linebuffered = linebuffered;
    }
    if (haveread) {
        netstream->c->canautoclose = 1;
    }else{
        netstream->c->canautoclose = 0;
    }
    return 0;
}

static int connectedevents(struct connection* c) {
    lua_State* l = (lua_State*)c->userdata;

    // ensure stack is large enough
    if (!lua_checkstack(l, 10)) {
        return haveluaerror(l, stackgrowfailure);
    }

    // push error handling function
    lua_pushcfunction(l, internaltracebackfunc());

    // push connect callback function
    char regname[500];
    snprintf(regname, sizeof(regname), "opencallback%p", c);
    regname[sizeof(regname)-1] = 0;
    lua_pushstring(l, regname);
    lua_gettable(l, LUA_REGISTRYINDEX);

    // check if we actually have a read callback:
    if (lua_type(l, -1) == LUA_TNIL) {
        lua_pop(l, 1);
        return 1;
    }

    // push reference to the connection:
    createnetstreamobj(l, c);

    // prompt callback:
    int result = lua_pcall(l, 1, 0, -3);
    if (result != 0) {
        printerror("Error: An error occured when calling blitwiz.net.open() open callback: %s", lua_tostring(l, -1));
        lua_pop(l, 2); // pop error message, error handler
        return 0;
    }
    lua_pop(l, 1); // pop error handler
    return 1;
}

static int readevents(struct connection* c, char* data, unsigned int datalength) {
    lua_State* l = (lua_State*)c->userdata;

    // attempt to grow stack first
    if (!lua_checkstack(l, 10)) {
        return haveluaerror(l, stackgrowfailure);
    }

    // push error handling function
    lua_pushcfunction(l, internaltracebackfunc());

    // read callback lua function
    char regname[500];
    snprintf(regname, sizeof(regname), "readcallback%p", c);
    regname[sizeof(regname)-1] = 0;
    lua_pushstring(l, regname);
    lua_gettable(l, LUA_REGISTRYINDEX);

    // check if we actually have a read callback:
    if (lua_type(l, -1) == LUA_TNIL) {
        lua_pop(l, 1);
        return 1;
    }

    // push reference to the connection:
    createnetstreamobj(l, c);

    // compose data parameter:
    luaL_Buffer b;
    luaL_buffinit(l, &b);
    luaL_addlstring(&b, data, datalength);
    luaL_pushresult(&b); // FIXME: how much stack space does this need?

    // prompt callback:
    int result = lua_pcall(l, 2, 0, -4);
    if (result != 0) {
        printerror("Error: An error occured when calling blitwiz.net.open() read callback: %s", lua_tostring(l, -1));
        lua_pop(l, 1); // pop error message, error handler
        return 0;
    }
    lua_pop(l, 1); // pop error handler

    return 1;
}

int luafuncs_netserver(lua_State* l) {
    // get parameters:
    if (lua_gettop(l) < 1 || lua_type(l, 1) != LUA_TNUMBER) {
        return haveluaerror(l, badargument1, 1, "blitwiz.net.server", "number", lua_strtype(l, 1));
    }
    int port = lua_tointeger(l, 1);
    if (port < 1 || port > 65535) {
        return haveluaerror(l, badargument2, 1, "blitwiz.net.server", "port not in valid range");
    }
    if (lua_gettop(l) < 2 || lua_type(l, 2) != LUA_TFUNCTION) {
        return haveluaerror(l, badargument1, 2, "blitwiz.net.server", "function", lua_strtype(l, 2));
    }

    // clean up stack from everything we don't want:
    if (lua_gettop(l) > 2) {
        lua_pop(l, lua_gettop(l)-2);
    }

    // create server listener:
    int result = listeners_Create(port, 0, l);
    if (!result) {
        if (port > 1024) {
            return haveluaerror(l, "Cannot start server at %d - is that port already in use?", port);
        }else{
            return haveluaerror(l, "Cannot start server at %d - is that port already in use?\nIMPORTANT: You used a port <= 1024. This program might need special privilegues to use this port.", port);
        }
    }
    char p[512];
    snprintf(p, sizeof(p), "serverlistenercallback%d", port);
    lua_pushstring(l, p);
    lua_insert(l, -2);
    lua_settable(l, LUA_REGISTRYINDEX);

    // return the port in case of success:
    lua_pushnumber(l, port);
    return 1;
}

int connectionevents(int port, int socket, const char* ip, void* sslptr, void* userdata) {
    lua_State* l = (lua_State*)userdata;

    // ensure stack is large enough
    if (!lua_checkstack(l, 10)) {
        return haveluaerror(l, stackgrowfailure);
    }

    // push the error handler:
    lua_pushcfunction(l, internaltracebackfunc());

    // get the callback:
    char p[512];
    snprintf(p, sizeof(p), "serverlistenercallback%d", port);
    lua_pushstring(l, p);
    lua_gettable(l, LUA_REGISTRYINDEX);

    // without a valid callback, there is nothing we could do:
    if (lua_type(l, -1) != LUA_TFUNCTION) {
        so_CloseSSLSocket(socket, &sslptr);
        return 1;
    }

    // push server port:
    lua_pushnumber(l, port);

    // create a new connection which we can pass on to the callback:
    struct luaidref* iptr = createnetstreamobj(l, NULL); // FIXME: this should be pcall'ed probably

    struct connection* c = ((struct luanetstream*)iptr->ref.ptr)->c;
    memset(c, 0, sizeof(*c));
    c->socket = socket;
    c->sslptr = sslptr;
    c->targetport = port;
    c->connected = 1;
    c->luarefcount = 1; // since we pass it to the callback now.
    c->lastreadtime = time_GetMilliseconds();
    c->error = -1;
    c->userdata = l;
    c->autoclosecallback = &clearconnectioncallbacks;
    c->iptype = IPTYPE_IPV6;
    c->inbuf = malloc(CONNECTIONINBUFSIZE);
    c->inbufsize = CONNECTIONINBUFSIZE;
    c->outbuf = malloc(CONNECTIONOUTBUFSIZE);
    c->outbufsize = CONNECTIONOUTBUFSIZE;
    if (!c->inbuf || !c->outbuf) {
        if (c->inbuf) {
            free(c->inbuf);
        }
        if (c->outbuf) {
            free(c->outbuf);
        }
        free(c);
        free(iptr->ref.ptr);
        lua_pop(l, 4); // error handler, callback, port, luaidref
        printwarning("Failed to allocate connection");
        return 0;
    }

    // add connection to list:
    c->next = connectionlist;
    connectionlist = c;

    // prompt callback:
    int result = lua_pcall(l, 2, 0, -4);
    if (result != 0) {
        printerror("Error: An error occured when calling blitwiz.net.server() callback: %s", lua_tostring(l, -1));
        lua_pop(l, 1); // pop error message, error handler
        return 0;
    }
    lua_pop(l, 1); // pop error handler
    return 1;
}

static int errorevents(struct connection* c, int error) {
    lua_State* l = (lua_State*)c->userdata;

    // ensure stack is large enough
    if (!lua_checkstack(l, 10)) {
        return haveluaerror(l, stackgrowfailure);
    }

    // push error handling function
    lua_pushcfunction(l, internaltracebackfunc());

    // read callback lua function
    char regname[500];
    snprintf(regname, sizeof(regname), "errorcallback%p", c);
    regname[sizeof(regname)-1] = 0;
    lua_pushstring(l, regname);
    lua_gettable(l, LUA_REGISTRYINDEX);

    // check if we actually have an error callback:
    if (lua_type(l, -1) == LUA_TNIL) {
        lua_pop(l, 1);
        return 1;
    }

    // push reference to the connection:
    createnetstreamobj(l, c);

    // push error message:
    switch (error) {
        case CONNECTIONERROR_INITIALISATIONFAILED:
            lua_pushstring(l, "Initialisation of stream failed");
            break;
        case CONNECTIONERROR_NOSUCHHOST:
            lua_pushstring(l, "No such host");
            break;
        case CONNECTIONERROR_CONNECTIONFAILED:
            lua_pushstring(l, "Failed to connect");
            break;
        case CONNECTIONERROR_CONNECTIONCLOSED:
            lua_pushstring(l, "Connection closed");
            break;
        case CONNECTIONERROR_CONNECTIONAUTOCLOSE:
            lua_pushstring(l, "Connection was auto-closed due to lack of activity and missing Lua references");
            break;
        default:
            lua_pushstring(l, "Unknown connection error");
            break;
    }

    // prompt callback:
    int result = lua_pcall(l, 2, 0, -4);
    if (result != 0) {
        printerror("Error: An error occured when calling blitwiz.net.open() close callback: %s", lua_tostring(l, -1));
        lua_pop(l, 1); // pop error message, error handler
        return 0;
    }
    lua_pop(l, 1); // pop error message

    return 1;
}

int luafuncs_ProcessNetEvents() {
    int result = listeners_CheckForConnections(&connectionevents);
    if (!result) {
        return 0;
    }
    result = connections_CheckAll(&connectedevents, &readevents, &errorevents);
    return result;
}
