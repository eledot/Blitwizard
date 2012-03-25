
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
#include "luafuncs_physics.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "file.h"

static lua_State* scriptstate = NULL;

void luastate_PrintStackDebug() {
    //print the contents of the Lua stack
    printf("Debug stack:\n");
    int m = lua_gettop(scriptstate);
    int i = 1;
    while (i <= m) {
        printf("%d (", i);
        switch (lua_type(scriptstate, i)) {
            case LUA_TNIL:
                printf("nil");
                break;
            case LUA_TSTRING:
                printf("string");
                break;
            case LUA_TNUMBER:
                printf("number");
                break;
            case LUA_TBOOLEAN:
                printf("boolean");
                break;
            case LUA_TUSERDATA:
                printf("userdata");
                break;
            case LUA_TFUNCTION:
                printf("function");
                break;
            default:
                printf("unknown");
                break;
        }
        printf("): ");
        switch (lua_type(scriptstate, i)) {
            case LUA_TNIL:
                printf("nil");
                break;
            case LUA_TSTRING:
                printf("\"%s\"", lua_tostring(scriptstate, i));
                break;
            case LUA_TNUMBER:
                printf("%f", lua_tonumber(scriptstate, i));
                break;
            case LUA_TBOOLEAN:
                if (lua_toboolean(scriptstate, i)) {
                    printf("true");
                }else{
                    printf("false");
                }
                break;
            case LUA_TUSERDATA:
                printf("0x%x", (unsigned int)lua_touserdata(scriptstate, i));
                break;
            case LUA_TFUNCTION:
                printf("<function>");
                break;
        }
        printf("\n");
        i++;
    }
}

static void luastate_CreatePhysicsTable(lua_State* l) {
    lua_newtable(l);
    lua_pushstring(l, "setGravity");
    lua_pushcfunction(l, &luafuncs_setGravity);
    lua_settable(l, -3);
    lua_pushstring(l, "createMovableObject");
    lua_pushcfunction(l, &luafuncs_createMovableObject);
    lua_settable(l, -3);
    lua_pushstring(l, "createStaticObject");
    lua_pushcfunction(l, &luafuncs_createStaticObject);
    lua_settable(l, -3);
    lua_pushstring(l, "restrictRotation");
    lua_pushcfunction(l, &luafuncs_restrictRotation);
    lua_settable(l, -3);
    lua_pushstring(l, "setShapeRectangle");
    lua_pushcfunction(l, &luafuncs_setShapeRectangle);
    lua_settable(l, -3);
    lua_pushstring(l, "setShapeCircle");
    lua_pushcfunction(l, &luafuncs_setShapeCircle);
    lua_settable(l, -3);
    lua_pushstring(l, "setShapeOval");
    lua_pushcfunction(l, &luafuncs_setShapeOval);
    lua_settable(l, -3);
    lua_pushstring(l, "setShapeEdges");
    lua_pushcfunction(l, &luafuncs_setShapeEdges);
    lua_settable(l, -3);
    lua_pushstring(l, "setMass");
    lua_pushcfunction(l, &luafuncs_setMass);
    lua_settable(l, -3);
    lua_pushstring(l, "setFriction");
    lua_pushcfunction(l, &luafuncs_setFriction);
    lua_settable(l, -3);
    lua_pushstring(l, "setRestitution");
    lua_pushcfunction(l, &luafuncs_setRestitution);
    lua_settable(l, -3);
    lua_pushstring(l, "setAngularDamping");
    lua_pushcfunction(l, &luafuncs_setAngularDamping);
    lua_settable(l, -3);
    lua_pushstring(l, "setLinearDamping");
    lua_pushcfunction(l, &luafuncs_setLinearDamping);
    lua_settable(l, -3);
    lua_pushstring(l, "getRotation");
    lua_pushcfunction(l, &luafuncs_getRotation);
    lua_settable(l, -3);
    lua_pushstring(l, "getPosition");
    lua_pushcfunction(l, &luafuncs_getPosition);
    lua_settable(l, -3);
    lua_pushstring(l, "warp");
    lua_pushcfunction(l, &luafuncs_warp);
    lua_settable(l, -3);
    lua_pushstring(l, "impulse");
    lua_pushcfunction(l, &luafuncs_impulse);
    lua_settable(l, -3);
    lua_pushstring(l, "ray");
    lua_pushcfunction(l, &luafuncs_ray);
    lua_settable(l, -3);
}

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
    lua_pushstring(l, "isImageLoaded");
    lua_pushcfunction(l, &luafuncs_isImageLoaded);
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
    lua_pushstring(l, "sleep");
    lua_pushcfunction(l, &luafuncs_sleep);
    lua_settable(l, -3);
}

static int openlib_blitwiz(lua_State* l) {
    static const struct luaL_Reg blitwizlib[] = { {NULL, NULL} };
    luaL_newlib(l, blitwizlib);
    return 1;
}

static const luaL_Reg libswithoutdebug[] = {
    {"_G", luaopen_base},
    {LUA_LOADLIBNAME, luaopen_package},
    {LUA_COLIBNAME, luaopen_coroutine},
    {LUA_TABLIBNAME, luaopen_table},
    {LUA_IOLIBNAME, luaopen_io},
    {LUA_OSLIBNAME, luaopen_os},
    {LUA_STRLIBNAME, luaopen_string},
    {LUA_BITLIBNAME, luaopen_bit32},
    {LUA_MATHLIBNAME, luaopen_math},
    {NULL, NULL}
};

static int luastate_AddStdFuncs(lua_State* l) {
    const luaL_Reg* lib = libswithoutdebug;
    while (lib->func) {
        luaL_requiref(l, lib->name, lib->func, 1);
        lua_pop(l, 1);  
        lib++;
    }
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

//The next two functions are stolen (slightly modified) from Lua 5 :)
//Lua is zlib-licensed as this engine,
//  Copyright (C) 1994-2011 Lua.org, PUC-Rio. All rights reserved.
//Lua does not export those symbols, which is why I copied them here

static lua_State *getthread (lua_State *L, int *arg) { //FROM LUA 5
    if (lua_isthread(L, 1)) {
        *arg = 1;
        return lua_tothread(L, 1);
    }else{
        *arg = 0;
        return L;
    }
}

static int debug_traceback (lua_State *L) { //FROM LUA 5
    int arg;
    lua_State *L1 = getthread(L, &arg);
    const char *msg = lua_tostring(L, arg + 1);
    if (msg == NULL && !lua_isnoneornil(L, arg + 1)) {  /* non-string 'msg'? */
        lua_pushvalue(L, arg + 1);  /* return it untouched */
    }else{
        int level = luaL_optint(L, arg + 2, (L == L1) ? 1 : 0);
        luaL_traceback(L, L1, msg, level);
    }
    return 1;
}


static lua_State* luastate_New() {
    lua_State* l = luaL_newstate();
    
    //standard libs
    lua_pushcfunction(l, &luastate_AddStdFuncs);
    lua_pcall(l, 0, 0, 0);

    //preserve debug.traceback in the registry for our own use
    lua_pushstring(l, "debug_traceback_preserved");
    lua_pushcfunction(l, &debug_traceback);
    lua_settable(l, LUA_REGISTRYINDEX);
    
    //own dofile/loadfile/print
    lua_pushcfunction(l, &luafuncs_loadfile);
    lua_setglobal(l, "loadfile");
    lua_pushcfunction(l, &luafuncs_dofile);
    lua_setglobal(l, "dofile");
    lua_pushcfunction(l, &luafuncs_print);
    lua_setglobal(l, "print");

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
    
    lua_pushstring(l, "physics");
    luastate_CreatePhysicsTable(l);
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
    lua_pushstring(l, "getcwd");
    lua_pushcfunction(l, &luafuncs_getcwd);
    lua_settable(l, -3);
    lua_pushstring(l, "exists");
    lua_pushcfunction(l, &luafuncs_exists);
    lua_settable(l, -3);
    lua_pushstring(l, "sysname");
    lua_pushcfunction(l, &luafuncs_sysname);
    lua_settable(l, -3);
    lua_pushstring(l, "sysversion");
    lua_pushcfunction(l, &luafuncs_sysversion);
    lua_settable(l, -3);

    //throw table "os" off the stack
    lua_pop(l, 1);

    char vstr[512];
    snprintf(vstr, sizeof(vstr), "Blitwizard %s based on Lua 5.2", VERSION);
    lua_pushstring(l, vstr);
    lua_setglobal(l, "_VERSION");
    
    return l;
}

static int luastate_DoFile(lua_State* l, const char* file, char** error) {
    int previoustop = lua_gettop(l);
    lua_pushcfunction(l, &gettraceback);
    lua_getglobal(l, "dofile"); //first, push function
    lua_pushstring(l, file); //then push file name as argument
    int ret = lua_pcall(l, 1, 0, -3); //call returned function by loadfile

    int returnvalue = 1;
    //process errors
    if (ret != 0) {
        const char* e = lua_tostring(l,-1);
        *error = NULL;
        if (e) {
            *error = strdup(e);
        }
        returnvalue = 0;
    }

    //clean up stack
    if (lua_gettop(scriptstate) > previoustop) {
        lua_pop(scriptstate, lua_gettop(scriptstate) - previoustop);
    }

    return returnvalue;
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
    if (args > 0) {
        lua_insert(scriptstate, -(args+1));
    }

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

    int previoustop = lua_gettop(scriptstate)-(args+2); // 2 = 1 (error func) + 1 (called func)

    //call function
    int i = lua_pcall(scriptstate, args, 0, -(args+2));

    //process errors
    int returnvalue = 1;    
    if (i != 0) {
        *error = NULL;
        if (i == LUA_ERRRUN || i == LUA_ERRERR) {
            const char* e = lua_tostring(scriptstate, -1);
            *error = NULL;
            if (e) {
                *error = strdup(e);
            }
        }else{
            if (i == LUA_ERRMEM) {
                *error = strdup("Out of memory");
            }else{
                *error = strdup("Unknown error");
            }
        }
        returnvalue = 0;
    }

    //clean up stack
    if (lua_gettop(scriptstate) > previoustop) {
        lua_pop(scriptstate, lua_gettop(scriptstate) - previoustop);
    }

    return returnvalue;
}
