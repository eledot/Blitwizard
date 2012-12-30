
/* blitwizard 2d engine - source code file

  Copyright (C) 2011-2012 Jonas Thiem

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

@author Jonas Thiem  (jonas.thiem@gmail.com)
@copyright 2011-2012
@license zlib
@module blitwizard

*/

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
#include "luafuncs_objectphysics.h"
#include "main.h"

// @type object

// Put the collision callback of the given object on stack
static void luafuncs_pushcollisioncallback(lua_State* l,
struct blitwizardobject* obj) {
    char funcname[200];
    snprintf(funcname, sizeof(funcname), "collisioncallback%p", obj);
    funcname[sizeof(funcname)-1] = 0;
    lua_pushstring(l, funcname);
    lua_gettable(l, LUA_REGISTRYINDEX);
}

// Attempt to trigger a user-defined collision callback for a given object.
// When no callback is set by the user or if the callback succeeds,
// 1 will be returned. 
// In case of a lua error in the callback, 0 will be returned and a
// traceback printed to stderr.
static int luafuncs_trycollisioncallback(struct blitwizardobject* obj, struct blitwizardobject* otherobj, double x, double y, double z, double normalx, double normaly, double normalz, double force, int* enabled, int use3d) {
    // get global lua state we use for blitwizard (no support for multiple
    // states as of now):
    lua_State* l = luastate_GetStatePtr();

    // obtain the collision callback:
    luafuncs_pushcollisioncallback(l, obj);

    // check if the collision callback is not nil (-> defined):
    if (lua_type(l, -1) != LUA_TNIL) {
        // we got a collision callback for this object -> call it
        lua_pushcfunction(l, (lua_CFunction)internaltracebackfunc());
        lua_insert(l, -2);

        // push all args:
        luafuncs_pushbobjidref(l, otherobj);
        lua_pushnumber(l, x);
        lua_pushnumber(l, y);
        if (use3d) {
            lua_pushnumber(l, z);
        }
        lua_pushnumber(l, normalx);
        lua_pushnumber(l, normaly);
        if (use3d) {
            lua_pushnumber(l, normalz);
        }
        lua_pushnumber(l, force);

        // Call the function:
        int ret = lua_pcall(l, 6+2*use3d, 1, -(8+2*use3d));
        if (ret != 0) {
            callbackerror(l, "<blitwizardobject>:onCollision", lua_tostring(l, -1));
            lua_pop(l, 2); // pop error string, error handling function
            return 0;
        } else {
            // evaluate return result...
            if (!lua_toboolean(l, -1)) {
                *enabled = 0;
            }

            // pop error handling function and return value:
            lua_pop(l, 2);
        }
    } else {
        // callback was nil and not defined by user
        lua_pop(l, 1); // pop the nil value
    }
    return 1;
}

static int luafuncs_trycollisioncallback3d(struct blitwizardobject* obj, struct blitwizardobject* otherobj, double x, double y, double z, double normalx, double normaly, double normalz, double force, int* enabled) {
    return luafuncs_trycollisioncallback(obj, otherobj, x, y, z, normalx, normaly, normalz, force, enabled, 1);
}

static int luafuncs_trycollisioncallback2d(struct blitwizardobject* obj, struct blitwizardobject* otherobj, double x, double y, double normalx, double normaly, double force, int* enabled) {
    return luafuncs_trycollisioncallback(obj, otherobj, x, y, 0, normalx, normaly, 0, force, enabled, 0);
}

// This function can throw lua out of memory errors (but no others) and should
// therefore be pcall'ed. Since we don't handle out of memory sanely anyway,
// it isn't pcalled for now: FIXME
// This function gets the information about two objects colliding, and will
// subsequently attempt to call both object's collision callbacks.
// 
// The user callbacks can decide that the collision shouldn't be handled,
// in which case this function will return 0. Otherwise, it will return 1.
// If a lua error happens in the user callbacks (apart from out of memory),
// it will instant-quit blitwizard with backtrace (it will never return).
int luafuncs_globalcollision2dcallback_unprotected(void* userdata, struct physicsobject* a, struct physicsobject* b, double x, double y, double normalx, double normaly, double force) {
    // we want to track if any of the callbacks wants to ignore the collision:
    int enabled = 1;

    // get the associated blitwizard objects to the collision objects:
    struct blitwizardobject* aobj = (struct blitwizardobject*)physics_GetObjectUserdata(a);
    struct blitwizardobject* bobj = (struct blitwizardobject*)physics_GetObjectUserdata(b);
    
    // call first object's callback:
    if (!luafuncs_trycollisioncallback2d(aobj, bobj, x, y, normalx, normaly, force, &enabled)) {
        // a lua error happened and backtrace was spilled out -> ignore and continue
    }

    // call second object's callback:
    if (!luafuncs_trycollisioncallback2d(bobj, aobj, x, y, -normalx, -normaly, force, &enabled)) {
        // a lua error happened in the callback was spilled out -> ignore and continue
    }

    // if any of the callbacks wants to ignore the collision, return 0:
    if (!enabled) {
        return 0;
    }
    return 1;
}

int luafuncs_globalcollision3dcallback_unprotected(void* userdata, struct physicsobject* a, struct physicsobject* b, double x, double y, double z, double normalx, double normaly, double normalz, double force) {
    // we want to track if any of the callbacks wants to ignore the collision:
    int enabled = 1;

    // get the associated blitwizard objects to the collision objects:
    struct blitwizardobject* aobj = (struct blitwizardobject*)physics_GetObjectUserdata(a);
    struct blitwizardobject* bobj = (struct blitwizardobject*)physics_GetObjectUserdata(b);

    // call first object's callback:
    if (!luafuncs_trycollisioncallback3d(aobj, bobj, x, y, z, normalx, normaly, normalz, force, &enabled)) {
        // a lua error happened and backtrace was spilled out -> ignore and continue
    }

    // call second object's callback:
    if (!luafuncs_trycollisioncallback3d(bobj, aobj, x, y, z, -normalx, -normaly, -normalz, force, &enabled)) {
        // a lua error happened in the callback was spilled out -> ignore and continue
    }

    // if any of the callbacks wants to ignore the collision, return 0:
    if (!enabled) {
        return 0;
    }
    return 1;
}

/// Enable the physics simulation on the given object and make it
/// collide with others for which the simulation is enabled.
// @function enableCollision
int luafuncs_enableCollision(lua_State* l, int movable) {
    struct blitwizardobject* obj = toblitwizardobject(l, 1),

    // validate: parameters need to be a list of shape info tables
    int argcount = lua_gettop(l)-1;
    if (argcount <= 0) {
        return haveluaerror(badargument1, 2,
        "blitwizard.object:enableCollision", "table", nil);
    } else {
        // check for args to be a table
        int i = 0;
        while (i < argcount) {
            if (lua_type(l, 2+i) != LUA_TTABLE &&
            lua_type(l, 2+i) != LUA_TNIL) {
                if (i == 0) {
                    return haveluaerror(badargument1, 2+i,
                    "blitwizard.object:enableCollision", "table",
                    lua_strtype(l, 2+i));
                } else {
                    return haveluaerror(badargument1, 2+i,
                    "blitwizard.object:enableCollision", "table or nil",
                    lua_strtype(l, 2+i));
                }
            }
            i++;
        }
    }

    // construct a shape list from the given shape tables:
    int i = 0;
    while (i < argcount) {
        lua_gettable(l, "type");
        if (lua_type(l, -1) != LUA_TSTRING) {
            return haveluaerror(badargument2, 2+i, "blitwizard.object:enableCollision", "Shape has invalid type: expected string");
        } else {
            const char* shapetype = lua_tostring(l, -1);

            lua_pop(l, 1);  // pop shapetype string
        }            
        i++;
    } 

    // create a physics object from the shapes
    if (!obj->physics) {
        obj->physics = malloc(sizeof(struct objectphysicsdata));
        memset(obj->physics);
    }
    if (obj->physics->object) {
        physics_DestroyObject(obj->physics->object);
    }
    obj->physics->object = physics_CreateObject();
 
    struct luaidref* ref = createphysicsobj(l);
    ((struct blitwizardobject*)ref->ref.ptr)->movable = 1;
    return 1;
}

int luafuncs_disableCollision(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    assert(obj->refcount > 0);

    if (!obj->physics) {
        // no physics info was set, ignore
        return 0;
    }

    if (obj->physics->object) {
        physics_DestroyWorld(obj->physics->object);
        obj->physics->object = NULL;
    }
    return 0;
}

int luafuncs_destroyObject(lua_State* l) {
    // destroy given physics object if possible
    struct blitwizardobject* obj = toblitwizardobject(l, 1);
    assert(obj->refcount > 0);

    obj->deleted = 1;

    if (obj->object) {
        // void collision callback
        char funcname[200];
        snprintf(funcname, sizeof(funcname), "collisioncallback%p", obj->object);
        funcname[sizeof(funcname)-1] = 0;
        lua_pushstring(l, funcname);
        lua_pushnil(l);
        lua_settable(l, LUA_REGISTRYINDEX);

        // delete physics body
        physics2d_DestroyObject(obj->object);
        obj->object = NULL;
    }
    return 0;
}

static void applyobjectsettings(struct toblitwizardobject* obj) {
    if (!obj->object) {
        return;
    }
    physics2d_SetRotationRestriction(obj->object, obj->rotationrestriction);
    physics2d_SetRestitution(obj->object, obj->restitution);
    physics2d_SetFriction(obj->object, obj->friction);
    physics2d_SetAngularDamping(obj->object, obj->angulardamping);
    physics2d_SetLinearDamping(obj->object, obj->lineardamping);
}

int luafuncs_impulse(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (!obj->movable) {
        lua_pushstring(l, "Impulse can be only applied to movable objects");
        return lua_error(l);
    }
    if (!obj->object) {
        lua_pushstring(l, "Physics object has no shape");
        return lua_error(l);
    }
    if (lua_type(l, 2) != LUA_TNUMBER) {
        lua_pushstring(l, "Second parameter is not a valid source x number");
        return lua_error(l);
    }
    if (lua_type(l, 3) != LUA_TNUMBER) {
        lua_pushstring(l, "Third parameter is not a valid source y number");
        return lua_error(l);
    }
    if (lua_type(l, 4) != LUA_TNUMBER) {
        lua_pushstring(l, "Fourth parameter is not a valid force x number");
        return lua_error(l);
    }
    if (lua_type(l, 5) != LUA_TNUMBER) {
        lua_pushstring(l, "Fifth parameter is not a valid force y number");
        return lua_error(l);
    }
    double sourcex = lua_tonumber(l, 2);
    double sourcey = lua_tonumber(l, 3);
    double forcex = lua_tonumber(l, 4);
    double forcey = lua_tonumber(l, 5);
    physics2d_ApplyImpulse(obj->object, forcex, forcey, sourcex, sourcey);
    return 0;
}

int luafuncs_ray(lua_State* l, int use3d) {
    if (lua_type(l, 1) != LUA_TNUMBER) {
        lua_pushstring(l, "First parameter is not a valid start x position");
        return lua_error(l);
    }
    if (lua_type(l, 2) != LUA_TNUMBER) {
        lua_pushstring(l, "Second parameter is not a valid start y position");
        return lua_error(l);
    }
    if (use3d) {
        if (lua_type(l, 3) != LUA_TNUMBER) {
            lua_pushstring(l, "Fourth parameter is not a valid start z position");
            return lua_error(l);
        }
    }
    if (lua_type(l, 3 + use3d) != LUA_TNUMBER) {
        lua_pushstring(l, "Third parameter is not a valid target x position");
        return lua_error(l);
    }
    if (lua_type(l, 4 + use3d) != LUA_TNUMBER) {
        lua_pushstring(l, "Fourth parameter is not a valid target y position");
        return lua_error(l);
    }
    if (use3d) {
        if (lua_type(l, 6) != LUA_TNUMBER) {
            lua_pushstring(l, "Fourth parameter is not a valid target z position");
            return lua_error(l);
        }
    }

    double startx = lua_tonumber(l, 1);
    double starty = lua_tonumber(l, 2);
    double startz;
    if (use3d) {
        startz = lua_tonumber(l, 3);
    }
    double targetx = lua_tonumber(l, 3+use3d);
    double targety = lua_tonumber(l, 4+use3d);
    double targetz;
    if (use3d) {
        targetz = lua_tonumber(l, 6);
    }

    struct physicsobject2d* obj;
    double hitpointx,hitpointy,hitpointz;
    double normalx,normaly,normalz;

    int returnvalue;
    if (use3d) {
        returnvalue = physics3d_Ray(main_DefaultPhysics2dPtr(), startx, starty, targetx, targety, &hitpointx, &hitpointy, &obj, &normalx, &normaly);
    } else {
        returnvalue = physics3d_Ray(main_DefaultPhysics2dPtr(), startx, starty, targetx, targety, &hitpointx, &hitpointy, &obj, &normalx, &normaly);
    }
    
    if (returnvalue) {
        // create a new reference to the (existing) object the ray has hit:
        luafuncs_pushbobjidref(l, (struct blitwizardobject*)physics2d_GetObjectUserdata(obj));

        // push the other information we also want to return:
        lua_pushnumber(l, hitpointx);
        lua_pushnumber(l, hitpointy);
        if (use3d) {
            lua_pushnumber(l, hitpointz);
        }
        lua_pushnumber(l, normalx);
        lua_pushnumber(l, normaly);
        if (use3d) {
            lua_pushnumber(l, normalz);
        }
        return 5+2*use3d;  // return it all
    }
    lua_pushnil(l);
    return 1;
}

int luafuncs_ray2d(lua_State* l) {
    return luafuncs_ray(l, 0);
}

int luafuncs_ray3d(lua_State* l) {
    return luafuncs_ray(l, 1);
}

int luafuncs_restrictRotation(lua_State* l) {
    struct blitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (lua_type(l, 2) != LUA_TBOOLEAN) {
        lua_pushstring(l, "Second parameter is not a valid rotation restriction boolean");
        return lua_error(l);
    }
    if (!obj->movable) {
        lua_pushstring(l, "Mass can be only set on movable objects");
        return lua_error(l);
    }
    obj->rotationrestriction = lua_toboolean(l, 2);
    applyobjectsettings(obj);
    return 0;
}

int luafuncs_setGravity(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (!obj->object) {
        lua_pushstring(l, "Physics object has no shape");
        return lua_error(l);
    }

    int set = 0;
    double gx,gy;
    if (lua_gettop(l) >= 3 && lua_type(l, 3) != LUA_TNIL) {
        if (lua_type(l, 2) != LUA_TNUMBER) {
            lua_pushstring(l, "Second parameter is not a valid gravity x number");
            return lua_error(l);
        }
        if (lua_type(l, 3) != LUA_TNUMBER) {
            lua_pushstring(l, "Third parameter is not a valid gravity y number");
            return lua_error(l);
        }
        gx = lua_tonumber(l, 2);
        gy = lua_tonumber(l, 3);
        set = 1;
    }
    if (set) {
        physics2d_SetGravity(obj->object, gx, gy);
    } else {
        physics2d_UnsetGravity(obj->object);
    }
    return 0;
}

int luafuncs_setMass(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (!obj->movable) {
        lua_pushstring(l, "Mass can be only set on movable objects");
        return lua_error(l);
    }
    if (lua_gettop(l) < 2 || lua_type(l, 2) != LUA_TNUMBER || lua_tonumber(l, 2) <= 0) {
        lua_pushstring(l, "Second parameter is not a valid mass number");
        return lua_error(l);
    }
    double centerx = 0;
    double centery = 0;
    double mass = lua_tonumber(l, 2);
    if (lua_gettop(l) >= 3 && lua_type(l, 3) != LUA_TNIL) {
        if (lua_type(l, 3) != LUA_TNUMBER) {
            lua_pushstring(l, "Third parameter is not a valid x center offset number");
            return lua_error(l);
        }
        centerx = lua_tonumber(l, 3);
    }
    if (lua_gettop(l) >= 4 && lua_type(l, 4) != LUA_TNIL) {
        if (lua_type(l, 4) != LUA_TNUMBER) {
            lua_pushstring(l, "Fourth parameter is not a valid y center offset number");
            return lua_error(l);
        }
        centery = lua_tonumber(l, 4);
    }
    if (!obj->object) {
        lua_pushstring(l, "Physics object has no shape");
        return lua_error(l);
    }
    physics2d_SetMass(obj->object, mass);
    physics2d_SetMassCenterOffset(obj->object, centerx, centery);
    return 0;
}

void transferbodysettings(struct physicsobject2d* oldbody, struct physicsobject2d* newbody) {
    double mass = physics2d_GetMass(oldbody);
    double massx,massy;
    physics2d_GetMassCenterOffset(oldbody, &massx, &massy);
    physics2d_SetMass(newbody, mass);
    physics2d_SetMassCenterOffset(newbody, massx, massy);
}

int luafuncs_warp(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (!obj->object) {
        lua_pushstring(l, "Physics object doesn't have a shape");
        return lua_error(l);
    }
    double warpx,warpy,warpangle;
    physics2d_GetPosition(obj->object, &warpx, &warpy);
    physics2d_GetRotation(obj->object, &warpangle);
    if (lua_gettop(l) >= 2 && lua_type(l, 2) != LUA_TNIL) {
        if (lua_type(l, 2) != LUA_TNUMBER) {
            lua_pushstring(l, "Second parameter not a valid warp x position number");
            return lua_error(l);
        }
        warpx = lua_tonumber(l, 2);
    }
    if (lua_gettop(l) >= 3 && lua_type(l, 3) != LUA_TNIL) {
        if (lua_type(l, 3) != LUA_TNUMBER) {
            lua_pushstring(l, "Third parameter not a valid warp y position number");
            return lua_error(l);
        }
        warpy = lua_tonumber(l, 3);
    }
    if (lua_gettop(l) >= 4 && lua_type(l, 4) != LUA_TNIL) {
        if (lua_type(l, 4) != LUA_TNUMBER) {
            lua_pushstring(l, "Fourth parameter not a valid warp angle number");
            return lua_error(l);
        }
        warpangle = lua_tonumber(l, 4);
    }
    physics2d_Warp(obj->object, warpx, warpy, warpangle);
    return 0;
}

int luafuncs_getPosition(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (!obj->object) {
        lua_pushstring(l, "Physics object doesn't have a shape");
        return lua_error(l);
    }
    double x,y;
    physics2d_GetPosition(obj->object, &x, &y);
    lua_pushnumber(l, x);
    lua_pushnumber(l, y);
    return 2;
}

int luafuncs_setRestitution(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (lua_type(l, 2) != LUA_TNUMBER) {
        lua_pushstring(l, "Second parameter not a valid restitution number");
        return lua_error(l);
    }
    obj->restitution = lua_tonumber(l, 2);
    applyobjectsettings(obj);
    return 0;
}

int luafuncs_setFriction(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (lua_type(l, 2) != LUA_TNUMBER) {
        lua_pushstring(l, "Second parameter not a valid friction number");
        return lua_error(l);
    }
    obj->friction = lua_tonumber(l, 2);
    applyobjectsettings(obj);
    return 0;
}

int luafuncs_setLinearDamping(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (lua_type(l, 2) != LUA_TNUMBER) {
        lua_pushstring(l, "Second parameter not a valid angular damping number");
        return lua_error(l);
    }
    obj->lineardamping = lua_tonumber(l, 2);
    applyobjectsettings(obj);
    return 0;
}

int luafuncs_setAngularDamping(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (lua_type(l, 2) != LUA_TNUMBER) {
        lua_pushstring(l, "Second parameter not a valid angular damping number");
        return lua_error(l);
    }
    obj->angulardamping = lua_tonumber(l, 2);
    applyobjectsettings(obj);
    return 0;
}

int luafuncs_getRotation(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (!obj->object) {
        lua_pushstring(l, "Physics object doesn't have a shape");
        return lua_error(l);
    }
    double angle;
    physics2d_GetRotation(obj->object, &angle);
    lua_pushnumber(l, angle);
    return 1;
}

int luafuncs_setShapeEdges(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (obj->object) {
        lua_pushstring(l, "Physics object already has a shape");
        return lua_error(l);
    }
    if (lua_gettop(l) < 2 || lua_type(l, 2) != LUA_TTABLE) {
        lua_pushstring(l, "Second parameter is not a valid edge list table");
        return lua_error(l);
    }
    if (obj->movable) {
        lua_pushstring(l, "This shape is not allowed for movable objects");
        return lua_error(l);
    }

    struct physicsobject2dedgecontext* context = physics2d_CreateObjectEdges_Begin(main_DefaultPhysics2dPtr(), obj, 0, obj->friction);

    int haveedge = 0;
    double d = 1;
    while (1) {
        lua_pushnumber(l, d);
        lua_gettable(l, 2);
        if (lua_type(l, -1) != LUA_TTABLE) {
            if (lua_type(l, -1) == LUA_TNIL && haveedge) {
                break;
            }
            lua_pushstring(l, "Edge list contains non-table value or is empty");
            physics2d_DestroyObject(physics2d_CreateObjectEdges_End(context));
            return lua_error(l);
        }
        haveedge = 1;

        double x1,y1,x2,y2;
        lua_pushnumber(l, 1);
        lua_gettable(l, -2);
        x1 = lua_tonumber(l, -1);
        lua_pop(l, 1);

        lua_pushnumber(l, 2);
        lua_gettable(l, -2);
        y1 = lua_tonumber(l, -1);
        lua_pop(l, 1);

        lua_pushnumber(l, 3);
        lua_gettable(l, -2);
        x2 = lua_tonumber(l, -1);
        lua_pop(l, 1);

        lua_pushnumber(l, 4);
        lua_gettable(l, -2);
        y2 = lua_tonumber(l, -1);
        lua_pop(l, 1);

        physics2d_CreateObjectEdges_Do(context, x1, y1, x2, y2);
        lua_pop(l, 1);
        d++;
    }

    struct physicsobject2d* oldobject = obj->object;

    obj->object = physics2d_CreateObjectEdges_End(context);
    if (!obj->object) {
        lua_pushstring(l, "Creation of the edges shape failed");
        return lua_error(l);
    }

    if (oldobject) {
        transferbodysettings(oldobject, obj->object);
        physics2d_DestroyObject(oldobject);
    }
    applyobjectsettings(obj);
    return 0;
}

int luafuncs_setShapeCircle(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (obj->object) {
        lua_pushstring(l, "Physics object already has a shape");
        return lua_error(l);
    }
    if (lua_gettop(l) < 2 || lua_type(l, 2) != LUA_TNUMBER || lua_tonumber(l, 2) <= 0) {
        lua_pushstring(l, "Not a valid circle radius number");
        return lua_error(l);
    }
    double radius = lua_tonumber(l, 2);
    struct physicsobject2d* oldobject = obj->object;
    obj->object = physics2d_CreateObjectOval(main_DefaultPhysics2dPtr(), obj, obj->movable, obj->friction, radius, radius);
    if (!obj->object) {
        lua_pushstring(l, "Failed to allocate shape");
        return lua_error(l);
    }

    if (oldobject) {
        transferbodysettings(oldobject, obj->object);
        physics2d_DestroyObject(oldobject);
    }
    applyobjectsettings(obj);
    return 0;
}


int luafuncs_setShapeOval(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (obj->object) {
        lua_pushstring(l, "Physics object already has a shape");
        return lua_error(l);
    }
    if (lua_gettop(l) < 2 || lua_type(l, 2) != LUA_TNUMBER || lua_tonumber(l, 2) <= 0) {
        lua_pushstring(l, "Not a valid oval width number");
        return lua_error(l);
    }
    if (lua_gettop(l) < 3 || lua_type(l, 3) != LUA_TNUMBER || lua_tonumber(l, 3) <= 0) {
        lua_pushstring(l, "Not a valid oval height number");
        return lua_error(l);
    }
    double width = lua_tonumber(l, 2);
    double height = lua_tonumber(l, 3);
    struct physicsobject2d* oldobject = obj->object;
    obj->object = physics2d_CreateObjectOval(main_DefaultPhysics2dPtr(), obj, obj->movable, obj->friction, width, height);
    if (!obj->object) {
        obj->object = oldobject;
        lua_pushstring(l, "Failed to allocate shape");
        return lua_error(l);
    }

    if (oldobject) {
        transferbodysettings(oldobject, obj->object);
        physics2d_DestroyObject(oldobject);
    }
    applyobjectsettings(obj);
    return 0;
}


int luafuncs_setCollisionCallback(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (!obj->object) {
        lua_pushstring(l, "Physics object doesn't have a shape");
        return lua_error(l);
    }
    if (lua_gettop(l) < 2 || lua_type(l, 2) != LUA_TFUNCTION) {
        return haveluaerror(l, badargument1, 2, "blitwiz.physics.setCollisionCallback", "function", lua_strtype(l, 2));
    }

    if (lua_gettop(l) > 2) {
        lua_pop(l, lua_gettop(l)-2);
    }

    char funcname[200];
    snprintf(funcname, sizeof(funcname), "collisioncallback%p", obj->object);
    funcname[sizeof(funcname)-1] = 0;
    lua_pushstring(l, funcname);
    lua_insert(l, -2);
    lua_settable(l, LUA_REGISTRYINDEX);

    return 0;
}


int luafuncs_setShapeRectangle(lua_State* l) {
    struct toblitwizardobject* obj = toblitwizardobject(l, 1);
    if (obj->deleted) {
        lua_pushstring(l, "Physics object was deleted");
        return lua_error(l);
    }
    if (obj->object) {
        lua_pushstring(l, "Physics object already has a shape");
        return lua_error(l);
    }
    if (lua_gettop(l) < 2 || lua_type(l, 2) != LUA_TNUMBER || lua_tonumber(l, 2) <= 0) {
        lua_pushstring(l, "Not a valid rectangle width number");
        return lua_error(l);
    }
    if (lua_gettop(l) < 3 || lua_type(l, 3) != LUA_TNUMBER || lua_tonumber(l, 3) <= 0) {
        lua_pushstring(l, "Not a valid rectangle height number");
        return lua_error(l);
    }
    double width = lua_tonumber(l, 2);
    double height = lua_tonumber(l, 3);
    struct physicsobject2d* oldobject = obj->object;
    obj->object = physics2d_CreateObjectRectangle(main_DefaultPhysics2dPtr(), obj, obj->movable, obj->friction, width, height);
    if (!obj->object) {
        lua_pushstring(l, "Failed to allocate shape");
        return lua_error(l);
    }

    if (oldobject) {
        transferbodysettings(oldobject, obj->object);
        physics2d_DestroyObject(oldobject);
    }
    applyobjectsettings(obj);
    return 0;
}

#endif

