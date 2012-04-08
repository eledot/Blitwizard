
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

#include "lua.h"
#include "luafuncs_net.h"

int luafuncs_netopen(lua_State* l) {
    if (lua_gettop(l) < 1 || lua_type(l, 1) != LUA_TTABLE) {
        lua_pushstring(l, "First argument is not a valid settings table");
        return lua_error(l);
    }
    lua_pushstring(l, "server");
    lua_gettable(l, -2);
    const char* p = lua_tostring(l, -1);
    if (!p) {
        lua_pushstring(l, "The settings table doesn't contain a valid target server name ('server' setting)");
        return lua_error(l);
    }
    char* server = strdup(p);
    lua_pop(l, 1); //pop server name string again
    lua_pushstring(l, "port");
    lua_gettable(l, -2);
    if (lua_type(l, -1) != LUA_TNUMBER) {
        lua_pushstring(l, "The settings table doesn't contain a valid target port number ('port'setting)");
        return lua_error(l);
    }
    int port = lua_tointeger(l, -1);
    lua_pop(l, 1); //pop port number again
    if (port < 1 || port > 65535) {
        lua_pushstring(l, "The port number exceeds the possible port range");
        return lua_error(l);
    }
    printf("Target is: %s:%d\n", server, port);
    free(server);
    return 0;
}

