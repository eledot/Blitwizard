
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

#ifndef BLItWIZARD_LUAFUNCS_PHYSICS_H_
#define BLITWIZARD_LUAFUNCS_PHYSICS_H_

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "luaheader.h"
#include "physics.h"
#include "objectphysicsdata.h"

int luafuncs_ray2d(lua_State* l);
int luafuncs_ray3d(lua_State* l);
int luafuncs_set2dGravity(lua_State* l);
int luafuncs_set3dGravity(lua_State* l);

#endif  // BLITWIZARD_LUAFUNCS_PHYSICS_H_

