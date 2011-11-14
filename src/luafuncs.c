
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

#include <stdlib.h>
#include <string.h>

#include "lua.h"
#include "lauxlib.h"

#include "luafuncs.h"
#include "graphics.h"

int drawingallowed = 0;

int luafuncs_loadfile(lua_State* l) {
	const char* p = lua_tostring(l,1);
	if (!p) {
		lua_pushstring(l, "First argument is not a file name string");
		return lua_error(l);
	}
	int r = luaL_loadfile(l, p);
	if (r != 0) {
		char errormsg[512];
		if (r == LUA_ERRFILE) {
			lua_pushstring(l, "Cannot open file");
			return lua_error(l);
		}
		if (r == LUA_ERRSYNTAX) {
			snprintf(errormsg,sizeof(errormsg),"Syntax error: %s",lua_tostring(l,-1));
			lua_pop(l, 1);
			lua_pushstring(l, errormsg);
			return lua_error(l);
		}
		return lua_error(l);
	}
	return 1;
}

int luafuncs_dofile(lua_State* l) {
	const char* p = lua_tostring(l,1);
	if (!p) {
		lua_pushstring(l,"First argument is not a file name string");
		return lua_error(l);
	}
	int i = lua_gettop(l);
	while (i > 1) {
		lua_pop(l,1);
		i--;
	}
	lua_pushstring(l, "loadfile");
	lua_gettable(l, LUA_GLOBALSINDEX); //first, push function
	lua_pushvalue(l, 1); //then push file name as argument
	lua_call(l, 1, 1); //call loadfile
	int previouscount = lua_gettop(l)-1; //minus the function on the stack which lua_pcall() removes
	int ret = lua_pcall(l, 0, LUA_MULTRET, 0); //call returned function by loadfile
	if (ret != 0) {
		return lua_error(l);
	}
	return lua_gettop(l)-previouscount;
}

int luafuncs_setWindow(lua_State* l) {
	if (lua_gettop(l) <= 0) {
		graphics_Quit();
		return 0;
	}
	int x = lua_tonumber(l,1);
	if (x <= 0) {
		lua_pushstring(l,"First argument is not a valid resolution width");
		return lua_error(l);
	}
	int y = lua_tonumber(l,2);
	if (y <= 0) {
		lua_pushstring(l,"First argument is not a valid resolution height");
		return lua_error(l);
	}
	int fullscreen = 0;
	if (lua_type(l,3) == LUA_TBOOLEAN) {
		fullscreen = lua_toboolean(l,3);
	}
	char defaulttitle[] = "blitwizard";
	const char* title = lua_tostring(l,4);
	if (!title) {
		title = defaulttitle;
	}
	const char* renderer = lua_tostring(l,5);
	char* error;
	if (!graphics_SetMode(x, y, fullscreen, 0, defaulttitle, renderer, &error)) {
		if (error) {
			lua_pushstring(l, error);
			free(error);
			return lua_error(l);
		}
		lua_pushstring(l, "Unknown error on setting mode");
		return lua_error(l);
	}
	return 0;
}

int luafuncs_loadImage(lua_State* l) {
	const char* p = lua_tostring(l,1);
	if (!p) {
		lua_pushstring(l, "First parameter is not a valid image name string");
		return lua_error(l);
	}
	graphics_PromptTextureLoading(p);
}


