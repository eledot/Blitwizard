
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

#ifndef BLItWIZARD_LUAFUNCS_OBJECTPHYSICS_H_
#define BLITWIZARD_LUAFUNCS_OBJECTPHYSICS_H_

#if (defined(USE_PHYSICS2D) || defined(USE_PHYSICS3D))

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "luaheader.h"
#include "physics.h"
#include "objectphysicsdata.h"

int luafuncs_enableStaticCollision(lua_State* l);
int luafuncs_enableMovableCollision(lua_State* l);
int luafuncs_destroyObject(lua_State* l);
int luafuncs_ray2d(lua_State* l);
int luafuncs_ray3d(lua_State* l);
int luafuncs_restrictRotation(lua_State* l);
int luafuncs_restrictRotationAroundAxis(lua_State* l);
int luafuncs_impulse(lua_State* l);
int luafuncs_setMass(lua_State* l);
int luafuncs_setRestitution(lua_State* l);
int luafuncs_setFriction(lua_State* l);
int luafuncs_setAngularDamping(lua_State* l);
int luafuncs_setCollisionCallback(lua_State* l);
int luafuncs_setLinearDamping(lua_State* l);
int luafuncs_setGravity(lua_State* l);

int luafuncs_freeObjectPhysicsData(struct objectphysicsdata* d);

struct physicsobject;
int luafuncs_globalcollision2dcallback_unprotected(void* userdata, struct physicsobject* a, struct physicsobject* b, double x, double y, double normalx, double normaly, double force);
int luafuncs_globalcollision3dcallback_unprotected(void* userdata, struct physicsobject* a, struct physicsobject* b, double x, double y, double z, double normalx, double normaly, double normalz, double force);

#endif  // USE_PHYSICS2D || USE_PHYSICS3D

// The following functions are available even when physics support
// is not compiled in:

int luafuncs_getRotation(lua_State* l);
int luafuncs_getPosition(lua_State* l);

void objectphysics_get2dRotation(struct blitwizardobject* obj,
double* angle);
void objectphysics_get3dRotation(struct blitwizardobject* obj,
double* qx, double* qy, double* qz, double* qrot);
void objectphysics_getPosition(struct blitwizardobject* obj,
double* x, double* y, double* z);
void objectphysics_warp2d(struct blitwizardobject* obj, double x, double y,
double angle, int anglespecified);
void objectphysics_warp3d(struct blitwizardobject* obj, double x, double y,
double z, double qx, double qy, double qz, double qrot, int anglespecified);

#endif  // BLITWIZARD_LUAFUNCS_OBJECTPHYSICS_H_

