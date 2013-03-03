
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

/// The blitwizard physics namespace contains various physics functions
// for collision tests and the like. You would handle most physics
// functionality by using functions of the
// @{blitwizard.object|blitwizard object} though.
// @author Jonas Thiem  (jonas.thiem@gmail.com)
// @copyright 2011-2013
// @license zlib
// @module blitwizard.physics

#if (defined(USE_PHYSICS2D) || defined(USE_PHYSICS3D))

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "luaheader.h"

#include "logging.h"
#include "luaerror.h"
#include "luastate.h"
#include "blitwizardobject.h"
#include "physics.h"
#include "objectphysicsdata.h"
#include "luafuncs_object.h"
#include "luafuncs_physics.h"
#include "main.h"

/// Do a ray collision test by shooting out a ray and checking where it hits
// in 2d realm.
// @function ray2d
// @tparam number startx Ray starting point, x coordinate (please note the starting point shouldn't be inside the collision shape of an object)
// @tparam number starty Ray starting point, y coordinate
// @tparam number targetx Ray target point, x coordinate
// @tparam number targety Ray target point, y coordinate
// @treturn userdata Returns a @{blitwizard.object|blitwizard object} if an object was hit by the ray (the closest object that was hit), or otherwise it returns nil
int luafuncs_ray2d(lua_State* l) {
    return luafuncs_ray(l, 0);
}

/// Do a ray collision test by shooting out a ray and checking where it hits
// in 3d realm.
// @function ray3d
// @tparam number startx Ray starting point, x coordinate (please note the starting point shouldn't be inside the collision shape of an object)
// @tparam number starty Ray starting point, y coordinate
// @tparam number startz Ray starting point, z coordinate
// @tparam number targetx Ray target point, x coordinate
// @tparam number targety Ray target point, y coordinate
// @tparam number targetz Ray target point, z coordinate
// @treturn userdata Returns a @{blitwizard.object|blitwizard object} if an object was hit by the ray (the closest object that was hit), or otherwise it returns nil
int luafuncs_ray3d(lua_State* l) {
    return luafuncs_ray(l, 1);
}

/// Set the world gravity for all 2d objects.
// @function set2dGravity
int luafuncs_set2dGravity(lua_State* l) {
    return 0;
}

/// Set the world gravity for all 3d objects.
// @function set3dGravity
int luafuncs_set3dGravity(lua_State* l) {
    return 0;
}

#endif  // USE_PHYSICS2D || USE_PHYSICS3D

