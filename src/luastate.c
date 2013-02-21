
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

#include "os.h"
#include "luaheader.h"
#include "luastate.h"
#include "luafuncs.h"
#include "physics.h"
#include "luafuncs_object.h"
#include "luafuncs_objectphysics.h"
#include "luafuncs_physics.h"
#include "luafuncs_net.h"
#include "luaerror.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "file.h"

// If USE_PHYSICS2D, or USE_PHYSICS3D respectively, isn't defined we wouldn't want to
// actually have a function call to luastate_register2dphysics_do/luastate_register3dphysics_do
// with the physics function pointer get evaluated by gcc.
// This way, we can avoid linker errors due to the physics functions not being present.
#ifdef USE_PHYSICS2D
#define luastate_register2dphysics luastate_register2dphysics_do
#else
#define luastate_register2dphysics(X, Y, Z) (luastate_register2dphysics_do(X, NULL, Z))
#endif
#ifdef USE_PHYSICS23
#define luastate_register3dphysics luastate_register2dphysics_do
#else
#define luastate_register3dphysics(X, Y, Z) (luastate_register2dphysics_do(X, NULL, Z))
#endif

static lua_State* scriptstate = NULL;

void* luastate_GetStatePtr() {
    return scriptstate;
}

void luastate_PrintStackDebug() {
    // print the contents of the Lua stack
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
                printf("<userdata %p>", lua_touserdata(scriptstate, i));
                break;
            case LUA_TFUNCTION:
                printf("<function>");
                break;
        }
        printf("\n");
        i++;
    }
}

// table must be on stack to which you want to set this function
void luastate_SetGCCallback(void* luastate, int tablestackindex, int (*callback)(void*)) {
    lua_State* l = luastate;
    lua_newtable(l);
    lua_pushstring(l, "__gc");
    lua_pushcfunction(l, (lua_CFunction)callback);
    lua_rawset(l, -3);
    if (tablestackindex < 0) {tablestackindex--;} // metatable is now at the top
    lua_setmetatable(l, tablestackindex);
    if (tablestackindex < 0) {tablestackindex++;}
}

int functionalitymissing_2dphysics(lua_State* l) {
    return haveluaerror(l, "%s", error_nophysics2d);
}

int functionalitymissing_3dphysics(lua_State* l) {
    return haveluaerror(l, "%s", error_nophysics3d);
}

void luastate_register2dphysics_do(lua_State* l, int (*func)(lua_State*), const char* name) {
    lua_pushstring(l, name);
#ifdef USE_PHYSICS2D
    lua_pushcfunction(l, func);
#else
    lua_pushcfunction(l, functionalitymissing_2dphysics);
#endif
    lua_settable(l, -3);
}

void luastate_register3dphysics_do(lua_State* l, int (*func)(lua_State*), const char* name) {
    lua_pushstring(l, name);
#ifdef USE_PHYSICS2D
    lua_pushcfunction(l, func);
#else
    lua_pushcfunction(l, functionalitymissing_3dphysics);
#endif
    lua_settable(l, -3);
}

static void luastate_CreatePhysicsTable(lua_State* l) {
    lua_newtable(l);
    luastate_register2dphysics(l, &luafuncs_set2dGravity, "set2dGravity");
    luastate_register2dphysics(l, &luafuncs_ray2d, "ray2d");
    luastate_register3dphysics(l, &luafuncs_set3dGravity, "set3dGravity");
    luastate_register3dphysics(l, &luafuncs_ray3d, "ray3d");
}

static void luastate_CreateNetTable(lua_State* l) {
    lua_newtable(l);
    lua_pushstring(l, "open");
    lua_pushcfunction(l, &luafuncs_netopen);
    lua_settable(l, -3);
    lua_pushstring(l, "server");
    lua_pushcfunction(l, &luafuncs_netserver);
    lua_settable(l, -3);
    lua_pushstring(l, "set");
    lua_pushcfunction(l, &luafuncs_netset);
    lua_settable(l, -3);
    lua_pushstring(l, "send");
    lua_pushcfunction(l, &luafuncs_netsend);
    lua_settable(l, -3);
    lua_pushstring(l, "close");
    lua_pushcfunction(l, &luafuncs_netclose);
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
    lua_pushstring(l, "unloadImage");
    lua_pushcfunction(l, &luafuncs_unloadImage);
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

static int luastate_AddBlitwizFuncs(lua_State* l) {
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

    // obtain the error first:
    const char* p = lua_tostring(l, -1);
    if (p) {
        unsigned int i = strlen(p);
        if (i >= sizeof(errormsg)) {i = sizeof(errormsg)-1;}
        memcpy(errormsg, p, i);
        errormsg[i] = 0;
    }
    lua_pop(l, 1);

    // add a line break if we can
    if (strlen(errormsg) + strlen("\n") < sizeof(errormsg)) {
        strcat(errormsg,"\n");
    }

    // call the original debug.traceback
    lua_pushstring(l, "debug_traceback_preserved");
    lua_gettable(l, LUA_REGISTRYINDEX);
    if (lua_type(l, -1) != LUA_TFUNCTION) { // make sure it is valid
        // oops.
        char invaliddebugtraceback[] = "OOPS: The debug traceback couldn't be generated:\n  Traceback function is not present or invalid (lua type is '";
        if (strlen(errormsg) + strlen(invaliddebugtraceback) < sizeof(errormsg)) {
            strcat(errormsg, invaliddebugtraceback);
        }

        // clarify on the invalid type of the function (which is not a func):
        char typebuf[64] = "";
        luatypetoname(lua_type(l, -1), typebuf, 16);
        strcat(typebuf, "')");
        if (strlen(errormsg) + strlen(typebuf) < sizeof(errormsg)) {
            strcat(errormsg, typebuf);
        }

        // push error:
        lua_pushstring(l, errormsg);
        return 1;
    }
    lua_call(l, 0, 1);

    // add the traceback as much as we can
    p = lua_tostring(l, -1);
    if (p) {
        int len = strlen(p);
        if (strlen(errormsg) + len >= sizeof(errormsg)) {
            len = sizeof(errormsg) - strlen(errormsg) - 1;
        }
        if (len > 0) {
            int offset = strlen(errormsg);
            memcpy(errormsg + offset, p, len);
            errormsg[offset + len] = 0;
        }
    }
    lua_pop(l, 1); // pop traceback

    // push the whole result
    lua_pushstring(l, errormsg);
    return 1;
}

void* internaltracebackfunc() {
    return &gettraceback;
}

void luastate_RememberTracebackFunc(lua_State* l) {
    lua_pushstring(l, "debug_traceback_preserved"); // push table index

    // obtain debug.traceback
    lua_getglobal(l, "debug");
    lua_pushstring(l, "traceback");
    lua_gettable(l, -2);

    // get rid of the table on the stack (position -2)
    lua_insert(l, -2);
    lua_pop(l, 1);

    // set it to the registry table
    lua_settable(l, LUA_REGISTRYINDEX);
}

static void luastate_VoidDebug(lua_State* l) {
    // void debug table
    lua_pushnil(l);
    lua_setglobal(l, "debug");

    // void debug library from package.loaded
    lua_getglobal(l, "package");
    lua_pushstring(l, "loaded");
    lua_gettable(l, -2);
    lua_pushstring(l, "debug");
    lua_pushnil(l);
    lua_settable(l, -3);

    // should we void binary loaders to avoid loading a debug.so?
}

static lua_State* luastate_New(void) {
    lua_State* l = luaL_newstate();

    lua_gc(l, LUA_GCSETPAUSE, 110);
    lua_gc(l, LUA_GCSETSTEPMUL, 300);

    // standard libs
    luaL_openlibs(l);
    luastate_RememberTracebackFunc(l);
    luastate_VoidDebug(l);
    luastate_AddBlitwizFuncs(l);

    // own dofile/loadfile/print
    lua_pushcfunction(l, &luafuncs_loadfile);
    lua_setglobal(l, "loadfile");
    lua_pushcfunction(l, &luafuncs_dofile);
    lua_setglobal(l, "dofile");
    lua_pushcfunction(l, &luafuncs_print);
    lua_setglobal(l, "print");

    // obtain the blitwiz lib
    lua_getglobal(l, "blitwiz");

    // blitwiz.setStep:
    lua_pushstring(l, "setStep");
    lua_pushcfunction(l, &luafuncs_setstep);
    lua_settable(l, -3);

    // blitwiz namespaces
    lua_pushstring(l, "graphics");
    luastate_CreateGraphicsTable(l);
    lua_settable(l, -3);

    lua_pushstring(l, "net");
    luastate_CreateNetTable(l);
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

    // we still have the module "blitwiz" on the stack here
    lua_pop(l, 1);

    // obtain math table
    lua_getglobal(l, "math");
    
    // math namespace extensions
    lua_pushstring(l, "trandom");
    lua_pushcfunction(l, &luafuncs_trandom);
    lua_settable(l, -3);

    // remove math table from stack
    lua_pop(l, 1);

    // obtain os table
    lua_getglobal(l, "os");

    // os namespace extensions
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

    // throw table "os" off the stack
    lua_pop(l, 1);

    // get "string" table for custom string functions
    lua_getglobal(l, "string");

    // set custom string functions
    lua_pushstring(l, "starts");
    lua_pushcfunction(l, &luafuncs_startswith);
    lua_settable(l, -3);
    lua_pushstring(l, "ends");
    lua_pushcfunction(l, &luafuncs_endswith);
    lua_settable(l, -3);
    lua_pushstring(l, "split");
    lua_pushcfunction(l, &luafuncs_split);
    lua_settable(l, -3);

    // throw table "string" off the stack
    lua_pop(l, 1);

    char vstr[512];
    char is64bit[] = " (64-bit binary)";
#ifndef _64BIT
    strcpy(is64bit, "");
#endif
    snprintf(vstr, sizeof(vstr), "Blitwizard %s based on Lua 5.2%s", VERSION, is64bit);
    lua_pushstring(l, vstr);
    lua_setglobal(l, "_VERSION");

    return l;
}

static int luastate_DoFile(lua_State* l, int argcount, const char* file, char** error) {
    int previoustop = lua_gettop(l);
    lua_pushcfunction(l, &gettraceback);
    lua_getglobal(l, "dofile"); // first, push function
    lua_pushstring(l, file); // then push file name as argument

    // in case of additional user-passed arguments, we need them too:
    if (argcount > 0) {
        // move file name argument in front of all arguments:
        lua_insert(l, -(argcount+3));

        // move function in front of all arguments (including file name):
        lua_insert(l, -(argcount+3));

        // move error function in front of all arguments + function:
        lua_insert(l, -(argcount+3));
    }

    int ret = lua_pcall(l, 1+argcount, 0, -(argcount+3)); // call returned function by loadfile

    int returnvalue = 1;
    // process errors
    if (ret != 0) {
        const char* e = lua_tostring(l,-1);
        *error = NULL;
        if (e) {
            *error = strdup(e);
        }
        returnvalue = 0;
    }

    // clean up stack (e.g. from return values or so)
    if (lua_gettop(scriptstate) > previoustop) {
        lua_pop(scriptstate, lua_gettop(scriptstate) - previoustop);
    }

    return returnvalue;
}

int luastate_DoInitialFile(const char* file, int argcount, char** error) {
    if (!scriptstate) {
        scriptstate = luastate_New();
        if (!scriptstate) {
            *error = strdup("Failed to initialize state");
            return 0;
        }
    }
    return luastate_DoFile(scriptstate, argcount, file, error);
}

int luastate_PushFunctionArgumentToMainstate_Bool(int yesno) {
    lua_pushboolean(scriptstate, yesno);
    return 1;
}

int luastate_PushFunctionArgumentToMainstate_String(const char* string) {
    if (!scriptstate) {
        scriptstate = luastate_New();
        if (!scriptstate) {
            return 0;
        }
    }
    lua_pushstring(scriptstate, string);
    return 1;
}

int luastate_PushFunctionArgumentToMainstate_Double(double i) {
    lua_pushnumber(scriptstate, i);
    return 1;
}

int luastate_CallFunctionInMainstate(const char* function, int args, int recursivetables, int allownil, char** error, int* functiondidnotexist) {
    // push error function
    lua_pushcfunction(scriptstate, &gettraceback);
    if (args > 0) {
        lua_insert(scriptstate, -(args+1));
    }

    // look up table components of our function name (e.g. namespace.func())
    int tablerecursion = 0;
    while (recursivetables && tablerecursion < 5) {
        unsigned int r = 0;
        int recursed = 0;
        while (r < strlen(function)) {
            if (function[r] == '.') {
                recursed = 1;
                // extract the component
                char* fp = malloc(r+1);
                if (!fp) {
                    *error = NULL;
                    // clean up stack again:
                    lua_pop(scriptstate, 1); // error func
                    lua_pop(scriptstate, args);
                    if (recursivetables > 0) {
                        // clean up recursive table left on stack
                        lua_pop(scriptstate, 1);
                    }
                    return 0;
                }
                memcpy(fp, function, r);
                fp[r] = 0;
                lua_pushstring(scriptstate, fp);
                function += r+1;
                free(fp);

                // lookup
                if (tablerecursion == 0) {
                    // lookup on global table
                    lua_getglobal(scriptstate, lua_tostring(scriptstate, -1));
                    lua_insert(scriptstate, -2);
                    lua_pop(scriptstate, 1);
                }else{
                    // lookup nested on previous table
                    lua_gettable(scriptstate, -2);

                    // dispose of previous table
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

    // lookup function normally if there was no recursion lookup:
    if (tablerecursion <= 0) {
        lua_getglobal(scriptstate, function);
    }else{
        // get the function from our recursive lookup
        lua_pushstring(scriptstate, function);
        lua_gettable(scriptstate, -2);
        // wipe out the table we got it from
        lua_insert(scriptstate,-2);
        lua_pop(scriptstate, 1);
    }

    // quit sanely if function is nil and we allowed this
    if (allownil && lua_type(scriptstate, -1) == LUA_TNIL) {
        // clean up stack again:
        lua_pop(scriptstate, 1); // error func
        lua_pop(scriptstate, args);
        if (recursivetables > 0) {
            // clean up recursive origin table left on stack
            lua_pop(scriptstate, 1);
        }
        // give back the info that it did not exist:
        if (functiondidnotexist) {
            *functiondidnotexist = 1;
        }
        return 1;
    }

    // function needs to be first, then arguments. -> correct order
    if (args > 0) {
        lua_insert(scriptstate, -(args+1));
    }

    int previoustop = lua_gettop(scriptstate)-(args+2); // 2 = 1 (error func) + 1 (called func)

    // call function
    int i = lua_pcall(scriptstate, args, 0, -(args+2));

    // process errors
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

    // clean up stack
    if (lua_gettop(scriptstate) > previoustop) {
        lua_pop(scriptstate, lua_gettop(scriptstate) - previoustop);
    }

    return returnvalue;
}

void luastate_GCCollect() {
    lua_gc(scriptstate, LUA_GCSTEP, 0);
}

