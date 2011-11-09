
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

#include "lua.h"
#include "lauxlib.h"
#include "luastate.h"
#include "luahelpers.h"

#include <stdlib.h>
#include <stdio.h>

static lua_State* scriptstate = NULL;

static lua_State* luastate_New() {
	lua_State* l = luaL_newstate();
	return l;
}

static int luastate_LoadFile(lua_State* l, const char* file, char** error) {
	
	return 1;
}

static int luastate_DoFile(lua_State* l, const char* file, char** error) {
	
	return 1;
}

int luastate_DoInitialFile(const char* file, char** error) {
	scriptstate = luastate_New();
	if (!scriptstate) {
		*error = strdup("Failed to initialize state!");
		return 0;
	}
	return luastate_DoFile(scriptstate, file, error);
}

int luastate_PushFunctionArgumentToMainstate_Bool(int yesno) {
	lua_pushboolean(scriptstate, yesno);
	return 1;
}

int luastate_PushFunctionArgumentToMainstate_String(const char* string) {
	lua_pushstring(scriptstate, string);
	return 1;
}


int luastate_CallFunctionInMainstate(const char* function, int args, char** error) {
	
}

