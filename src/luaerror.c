
/* blitwizard game engine - source code file

  Copyright (C) 2011-2013 Jonas Thiem

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
#include <stdarg.h>
#include <stdio.h>

#include "luaheader.h"
#include "luaerror.h"

char badargument1[] = "bad argument #%d to `%s` (%s expected, got %s)";
char badargument2[] = "bad argument #%d to `%s`: %s";
char stackgrowfailure[] = "Cannot grow stack size";

int haveluaerror(lua_State* l, const char* fmt, ...) {
    char printline[2048];
    va_list a;
    va_start(a, fmt);
    vsnprintf(printline, sizeof(printline)-1, fmt, a);
    printline[sizeof(printline)-1] = 0;
    va_end(a);
    lua_pushstring(l, printline);
    return lua_error(l);
}

void luatypetoname(int type, char* buf, size_t bufsize) {
    switch (type) {
        case LUA_TNIL:
            strncpy(buf, "nil", bufsize);
            break;
        case LUA_TFUNCTION:
            strncpy(buf, "function", bufsize);
            break;
        case LUA_TSTRING:
            strncpy(buf, "string", bufsize);
            break;
        case LUA_TNUMBER:
            strncpy(buf, "number", bufsize);
            break;
        case LUA_TBOOLEAN:
            strncpy(buf, "boolean", bufsize);
            break;
        case LUA_TTABLE:
            strncpy(buf, "table", bufsize);
            break;
        case LUA_TLIGHTUSERDATA:
            strncpy(buf, "lightuserdata",bufsize);
            break;
        case LUA_TUSERDATA:
            strncpy(buf, "userdata", bufsize);
            break;
        case LUA_TTHREAD:
            strncpy(buf, "thread", bufsize);
            break;
        case LUA_NUMTAGS:
            strncpy(buf, "numtag", bufsize);
            break;
        default:
            strncpy(buf, "unknown", bufsize);
            break;
    }
}

static char strtypebuf[64];
const char* lua_strtype(lua_State* l, int stack) {
    int type = lua_type(l, stack);
    luatypetoname(type, strtypebuf, sizeof(strtypebuf));
    return strtypebuf;
}

void callbackerror(lua_State* l, const char* function, const char* error, ...) {

}

