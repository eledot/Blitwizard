
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

#ifndef BLITWIZARD_LUAFUNCS_H_
#define BLITWIZARD_LUAFUNCS_H_

#include "luaheader.h"

// os:
int luafuncs_getcwd(lua_State* l);
int luafuncs_chdir(lua_State* l);
int luafuncs_isdir(lua_State* l);
int luafuncs_exists(lua_State* l);
int luafuncs_ls(lua_State* l);
int luafuncs_openConsole(lua_State* l);
int luafuncs_exit(lua_State* l);
int luafuncs_sysname(lua_State* l);
int luafuncs_sysversion(lua_State* l);

// blitwiz.*:
int luafuncs_setstep(lua_State* l);

// Base:
int luafuncs_loadfile(lua_State* l);
int luafuncs_dofile(lua_State* l);
int luafuncs_print(lua_State* l);

// Time:
int luafuncs_getTime(lua_State* l);
int luafuncs_sleep(lua_State* l);

// Graphics:
int luafuncs_getRendererName(lua_State* l);
int luafuncs_setWindow(lua_State* l);
int luafuncs_loadImage(lua_State* l);
int luafuncs_loadImageAsync(lua_State* l);
int luafuncs_getImageSize(lua_State* l);
int luafuncs_getWindowSize(lua_State* l);
int luafuncs_drawImage(lua_State* l);
int luafuncs_drawRectangle(lua_State* l);
int luafuncs_getDisplayModes(lua_State* l);
int luafuncs_getDesktopDisplayMode(lua_State* l);
int luafuncs_isImageLoaded(lua_State* l);
int luafuncs_unloadImage(lua_State* l);

// Sound:
int luafuncs_getBackendName(lua_State* l);
int luafuncs_play(lua_State* l);
int luafuncs_playing(lua_State* l);
int luafuncs_stop(lua_State* l);
int luafuncs_adjust(lua_State* l);

// Strings:
int luafuncs_startswith(lua_State* l);
int luafuncs_endswith(lua_State* l);
int luafuncs_split(lua_State* l);

// Math:
int luafuncs_trandom(lua_State* l);

#endif  // BLITWIZARD_LUAFUNCS_H_

