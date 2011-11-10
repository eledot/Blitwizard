
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
#include "lualib.h"
#include "luastate.h"
#include "luahelpers.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "file.h"

static lua_State* scriptstate = NULL;

static lua_State* luastate_New() {
	lua_State* l = luaL_newstate();
	
	//standard libs
	luaopen_base(l);
	luaopen_string(l);
	luaopen_table(l);
	luaopen_math(l);
	luaopen_os(l);
	
	//own dofile/loadfile	
	lua_pushcfunction(l, &luahelpers_loadfile);
	lua_setglobal(l,"loadfile");
	lua_pushcfunction(l, &luahelpers_dofile);
	lua_setglobal(l,"dofile");
	return l;
}

static int luastate_DoFile(lua_State* l, const char* file, char** error) {
	lua_pushstring(l, "dofile");
	lua_gettable(l, LUA_GLOBALSINDEX); //first, push function
	lua_pushstring(l, file); //then push file name as argument
	int ret = lua_pcall(l, 1, 0, 0); //call returned function by loadfile
	if (ret != 0) {
		const char* e = lua_tostring(l,-1);
		*error = NULL;
		if (e) {
			*error = strdup(e);
		}
		return 0;
	}
	return 1;
}

int luastate_DoInitialFile(const char* file, char** error) {
	scriptstate = luastate_New();
	if (!scriptstate) {
		*error = strdup("Failed to initialize state");
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

