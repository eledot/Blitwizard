
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
#include "luafuncs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "file.h"

static lua_State* scriptstate = NULL;

static void luastate_CreateGraphicsTable(lua_State* l) {
	lua_newtable(l);
	lua_pushstring(l, "getRendererName");
	lua_pushcfunction(l, &luafuncs_getRendererName);
	lua_settable(l, -3);
	lua_pushstring(l, "setWindow");
	lua_pushcfunction(l, &luafuncs_setWindow);
	lua_settable(l, -3);
	lua_pushstring(l, "loadImage");
	lua_pushcfunction(l, &luafuncs_loadImage);
	lua_settable(l, -3);
	lua_pushstring(l, "loadImageAsync");
	lua_pushcfunction(l, &luafuncs_loadImageAsync);
	lua_settable(l, -3);
	lua_pushstring(l, "getImageSize");
	lua_pushcfunction(l, &luafuncs_getImageSize);
	lua_settable(l, -3);
	lua_pushstring(l, "getWindowSize");
	lua_pushcfunction(l, &luafuncs_getWindowSize);
	lua_settable(l, -3);
	lua_pushstring(l, "drawImage");
	lua_pushcfunction(l, &luafuncs_drawImage);
	lua_settable(l, -3);
	lua_pushstring(l, "drawRectangle");
	lua_pushcfunction(l, &luafuncs_drawRectangle);
	lua_settable(l, -3);
	lua_pushstring(l, "getDisplayModes");
	lua_pushcfunction(l, &luafuncs_getDisplayModes);
	lua_settable(l, -3);
	lua_pushstring(l, "getDesktopDisplayMode");
	lua_pushcfunction(l, &luafuncs_getDesktopDisplayMode);
	lua_settable(l, -3);
}

static void luastate_CreateSoundTable(lua_State* l) {
	lua_newtable(l);
	lua_pushstring(l, "play");
	lua_pushcfunction(l, &luafuncs_play);
	lua_settable(l, -3);
	lua_pushstring(l, "adjust");
	lua_pushcfunction(l, &luafuncs_adjust);
	lua_settable(l, -3);
    lua_pushstring(l, "stop");
    lua_pushcfunction(l, &luafuncs_stop);
	lua_settable(l, -3);
	lua_pushstring(l, "playing");
    lua_pushcfunction(l, &luafuncs_playing);
	lua_settable(l, -3);
	lua_pushstring(l, "getBackendName");
	lua_pushcfunction(l, &luafuncs_getBackendName);
	lua_settable(l, -3);
}

static void luastate_CreateTimeTable(lua_State* l) {
	lua_newtable(l);
	lua_pushstring(l, "getTime");
	lua_pushcfunction(l, &luafuncs_getTime);
	lua_settable(l, -3);
}

static int openlib_blitwiz(lua_State* l) {
	static const struct luaL_Reg blitwizlib[] = { {NULL, NULL} };
	luaL_newlib(l, blitwizlib);
	return 1;
}

static int luastate_AddStdFuncs(lua_State* l) {
	luaL_openlibs(l);
	luaL_requiref(l, "blitwiz", openlib_blitwiz, 1);
	lua_pop(l, 1);	
	return 0;
}

char* luastate_GetPreferredAudioBackend() {
	lua_getglobal(scriptstate, "audiobackend");
	const char* p = lua_tostring(scriptstate, -1);
	char* s = NULL;
	if (p) {
		s = strdup(p);
	}
	lua_pop(scriptstate, 1);
	return s;
}

int luastate_GetWantFFmpeg() {
	lua_getglobal(scriptstate, "useffmpegaudio");
	if (lua_type(scriptstate, -1) == LUA_TBOOLEAN) {
		int i = lua_toboolean(scriptstate, -1);
		lua_pop(scriptstate, 1);
		if (i) {
			return 1;
		}
		return 0;
	}
	return 1;
}

static int gettraceback(lua_State* l) {
	char errormsg[2048] = "";
	const char* p = lua_tostring(l, -1);
	if (p) {
		unsigned int i = strlen(p);
		if (i >= sizeof(errormsg)) {i = sizeof(errormsg)-1;}
		memcpy(errormsg, p, i);
		errormsg[i] = 0;
	}
	lua_pop(l, 1);

	if (strlen(errormsg) + strlen("\n") < sizeof(errormsg)) {
		strcat(errormsg,"\n");
	}

	lua_pushstring(l, "debug_traceback_preserved");
	lua_gettable(l, LUA_REGISTRYINDEX);
	lua_call(l, 0, 1);

	p = lua_tostring(l, -1);
	if (p) {
		if (strlen(errormsg) + strlen(p) < sizeof(errormsg)) {
			strcat(errormsg, p);
		}
	}

	lua_pushstring(l, errormsg);
	return 1;
}


static lua_State* luastate_New() {
	lua_State* l = luaL_newstate();
	
	//standard libs
	lua_pushcfunction(l, &luastate_AddStdFuncs);
	lua_pcall(l, 0, 0, 0);

	//preserve debug.traceback in the registry for our own use
	lua_getglobal(l, "debug");
	lua_pushstring(l, "traceback");
	lua_gettable(l, -2);
	lua_pushstring(l, "debug_traceback_preserved");
	lua_insert(l, -2);
	lua_settable(l, LUA_REGISTRYINDEX);
	lua_pop(l, 1);
	
	//own dofile/loadfile	
	lua_pushcfunction(l, &luafuncs_loadfile);
	lua_setglobal(l, "loadfile");
	lua_pushcfunction(l, &luafuncs_dofile);
	lua_setglobal(l, "dofile");

	//obtain the blitwiz lib
	lua_getglobal(l, "blitwiz");

	//blitwiz namespace
	lua_pushstring(l, "graphics");
	luastate_CreateGraphicsTable(l);
	lua_settable(l, -3);
	
	lua_pushstring(l, "sound");
	luastate_CreateSoundTable(l);
	lua_settable(l, -3);
	
	lua_pushstring(l, "callback");
	lua_newtable(l);
		lua_pushstring(l, "event");
		lua_newtable(l);
		lua_settable(l,  -3);
	lua_settable(l, -3);

	lua_pushstring(l, "time");
	luastate_CreateTimeTable(l);
	lua_settable(l, -3);

	//we still have the module "blitwiz" on the stack here
	lua_pop(l, 1);

	//obtain os table
	lua_getglobal(l, "os");

	//os namespace extensions
	lua_pushstring(l, "exit");
	lua_pushcfunction(l, &luafuncs_exit);
	lua_settable(l, -3);
	lua_pushstring(l, "chdir");
	lua_pushcfunction(l, &luafuncs_chdir);
	lua_settable(l, -3);
	lua_pushstring(l, "openConsole");
	lua_pushcfunction(l, &luafuncs_openConsole);
	lua_settable(l, -3);
	lua_pushstring(l, "ls");
	lua_pushcfunction(l, &luafuncs_ls);
	lua_settable(l, -3);
	lua_pushstring(l, "isdir");
	lua_pushcfunction(l, &luafuncs_isdir);
	lua_settable(l, -3);
    lua_pushstring(l, "exists");
    lua_pushcfunction(l, &luafuncs_exists);
    lua_settable(l, -3);

	//throw table "os" off the stack
	lua_pop(l, 1);
	
	return l;
}

static int luastate_DoFile(lua_State* l, const char* file, char** error) {
	lua_pushcfunction(l, &gettraceback);
	lua_getglobal(l, "dofile"); //first, push function
	lua_pushstring(l, file); //then push file name as argument
	int ret = lua_pcall(l, 1, 0, -3); //call returned function by loadfile
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
	if (!scriptstate) {
		scriptstate = luastate_New();
		if (!scriptstate) {
			*error = strdup("Failed to initialize state");
			return 0;
		}
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

int luastate_PushFunctionArgumentToMainstate_Double(double i) {
	lua_pushnumber(scriptstate, i);
	return 1;
}

int luastate_CallFunctionInMainstate(const char* function, int args, int recursivetables, int allownil, char** error) {
	//push error function
	lua_pushcfunction(scriptstate, &gettraceback);

	//look up table components of our function name (e.g. namespace.func())
	int tablerecursion = 0;
	while (recursivetables && tablerecursion < 5) {
		unsigned int r = 0;
		int recursed = 0;
		while (r < strlen(function)) {
			if (function[r] == '.') {
				recursed = 1;
				//extract the component
				char* fp = malloc(r+1);
				if (!fp) {
					*error = NULL;
					//clean up stack again:
					lua_pop(scriptstate, 1); //error func
					lua_pop(scriptstate, args);
					if (recursivetables > 0) {
						//clean up recursive table left on stack
						lua_pop(scriptstate, 1);
					}
					return 0;
				}
				memcpy(fp, function, r);
				fp[r] = 0;
				lua_pushstring(scriptstate, fp);
				function += r+1;
				free(fp);

				//lookup
				if (tablerecursion == 0) {
					//lookup on global table
					lua_getglobal(scriptstate, lua_tostring(scriptstate, -1));
					lua_insert(scriptstate, -2);
					lua_pop(scriptstate, 1);
				}else{
					//lookup nested on previous table
					lua_gettable(scriptstate, -2);
					
					//dispose of previous table
					lua_insert(scriptstate, -2);
					lua_pop(scriptstate, 1);
				}
				break;
			}
			r++;
		}
		if (recursed) {
			tablerecursion++;
		}else{
			break;
		}
	}

	//lookup function normally if there was no recursion lookup:
	if (tablerecursion <= 0) {
		lua_getglobal(scriptstate, function);
	}else{
		//get the function from our recursive lookup
		lua_pushstring(scriptstate, function);
		lua_gettable(scriptstate, -2);
		//wipe out the table we got it from
		lua_insert(scriptstate,-2);
		lua_pop(scriptstate, 1);
	}

	//quit sanely if function is nil and we allowed this
	if (allownil && lua_type(scriptstate, -1) == LUA_TNIL) {
		//clean up stack again:
		lua_pop(scriptstate, 1); //error func
		lua_pop(scriptstate, args);
		if (recursivetables > 0) {
			//clean up recursive origin table left on stack
			lua_pop(scriptstate, 1);
  	    }
		return 1;
	}
	
	//function needs to be first, then arguments. -> correct order
	if (args > 0) {
        lua_insert(scriptstate, -(args+1));
    }

	//call function
	int i = lua_pcall(scriptstate, args, 0, -(args+2));
	if (i != 0) {
		if (i == LUA_ERRRUN) {
			const char* e = lua_tostring(scriptstate, -1);
			*error = NULL;
			if (e) {
				*error = strdup(e);
			}
			return 0;
		}
		if (i == LUA_ERRMEM) {
			*error = strdup("Out of memory");
			return 0;
		}
		*error = strdup("Unknown error");
		return 0;
	}
	return 1;
}
