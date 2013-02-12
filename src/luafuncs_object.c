
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

/// Blitwizard namespace containing the generic
// @{blitwizard.object|blitwizard game entity object} and various sub
// namespaces for @{blitwizard.physics|physics},
// @{blitwizard.graphics|graphics} and more.
// @author Jonas Thiem  (jonas.thiem@gmail.com)
// @copyright 2011-2013
// @license zlib
// @module blitwizard

#include <stdlib.h>
#include <string.h>

#include "os.h"
#ifdef USE_SDL_GRAPHICS
#include "SDL.h"
#endif
#include "graphicstexture.h"
#include "graphics.h"
#include "luaheader.h"
#include "luastate.h"
#include "luaerror.h"
#include "luafuncs.h"
#include "blitwizardobject.h"
#include "luafuncs_object.h"
#include "luafuncs_objectgraphics.h"
#include "luafuncs_objectphysics.h"

struct blitwizardobject* objects = NULL;
struct blitwizardobject* deletedobjects = NULL;

/// Blitwizard object which represents an 'entity' in the game world
// with visual representation, behaviour code and collision shape.
// @type object

void cleanupobject(struct blitwizardobject* o) {
    luafuncs_objectgraphics_unload(o);
#if (defined(USE_PHYSICS2D) || defined(USE_PHYSICS3D))
    if (o->physics) {
        luafuncs_freeObjectPhysicsData(o->physics);
        o->physics = NULL;
    }
#endif
}

static int garbagecollect_blitwizobjref(lua_State* l) {
    // we need to decrease our reference count of the
    // blitwizard object we referenced to.

    // get id reference to object
    struct luaidref* idref = lua_touserdata(l, -1);

    if (!idref || idref->magic != IDREF_MAGIC
    || idref->type != IDREF_BLITWIZARDOBJECT) {
        // either wrong magic (-> not a luaidref) or not a blitwizard object
        lua_pushstring(l, "internal error: invalid blitwizard object ref");
        lua_error(l);
        return 0;
    }

    // it is a valid blitwizard object, decrease ref count:
    struct blitwizardobject* o = idref->ref.bobj;
    o->refcount--;

    // if it's already deleted and ref count is zero, remove it
    // entirely and free it:
    if (o->deleted && o->refcount <= 0) {
        cleanupobject(o);

        // remove object from the list
        if (o->prev) {
            // adjust prev object to have new next pointer
            o->prev->next = o->next;
        } else {
            // was first object in deleted list -> adjust list start pointer
            deletedobjects = o->next;
        }
        if (o->next) {
            // adjust next object to have new prev pointer
            o->next->prev = o->prev;
        }

        // free object
        free(o);
    }
    return 0;
}

void luafuncs_pushbobjidref(lua_State* l, struct blitwizardobject* o) {
    // create luaidref userdata struct which points to the blitwizard object
    struct luaidref* ref = lua_newuserdata(l, sizeof(*ref));
    memset(ref, 0, sizeof(*ref));
    ref->magic = IDREF_MAGIC;
    ref->type = IDREF_BLITWIZARDOBJECT;
    ref->ref.bobj = o;
    luastate_SetGCCallback(l, -1, (int (*)(void*))&garbagecollect_blitwizobjref);
    o->refcount++;
}

struct blitwizardobject* toblitwizardobject(lua_State* l, int index, int arg, const char* func) {
    if (lua_type(l, index) != LUA_TUSERDATA) {
        haveluaerror(l, badargument1, arg, func, "blitwizard object", lua_strtype(l, index));
    }
    if (lua_rawlen(l, index) != sizeof(struct luaidref)) {
        haveluaerror(l, badargument2, arg, func, "not a valid blitwizard object");
    }
    struct luaidref* idref = lua_touserdata(l, index);
    if (!idref || idref->magic != IDREF_MAGIC
    || idref->type != IDREF_BLITWIZARDOBJECT) {
        haveluaerror(l, badargument2, arg, func, "not a valid blitwizard object");
    }
    struct blitwizardobject* o = idref->ref.bobj;
    if (o->deleted) {
        haveluaerror(l, badargument2, arg, func, "this blitwizard object was deleted");
    }
    return o;
}

/// Create a new blitwizard object which is represented as a 2d or
// 3d object in the game world.
// 2d objects are on a separate 2d plane, and 3d objects are inside
// the 3d world.
// Objects can have behaviour and collision info attached and move
// around. They are what eventually makes the action in your game!
// @function new
// @tparam boolean 3d specify 'true' if you wish this object to be a 3d object, or 'false' if you want it to be a flat 2d object
// @tparam string resource (optional) if you specify the file path to a resource here (optional), this resource will be loaded and used as a visual representation for the object. The resource must be a supported graphical object, e.g. an image (.png) or a 3d model (.mesh). You can also specify nil here if you don't want any resource to be used.
// @tparam function behaviour (optional) the behaviour function which will be executed immediately after object creation.
// @treturn userdata Returns a @{blitwizard.object|blitwizard object}
int luafuncs_object_new(lua_State* l) {
    // first argument needs to be 2d/3d boolean:
    if (lua_type(l, 1) != LUA_TBOOLEAN) {
        return haveluaerror(l, badargument1, 1, "blitwizard.object:new",
        "boolean", lua_strtype(l, 1));
    }
    int is3d = lua_toboolean(l, 1);

    // second argument, if present, needs to be the resource:
    const char* resource = NULL;
    if (lua_gettop(l) >= 2 && lua_type(l, 2) != LUA_TNIL) {
        if (lua_type(l, 2) != LUA_TSTRING) {
            return haveluaerror(l, badargument1, 2, "blitwizard.object:new",
            "string", lua_strtype(l, 2));
        }
        resource = lua_tostring(l, 2);
    }

    // create new object
    struct blitwizardobject* o = malloc(sizeof(*o));
    if (!o) {
        return haveluaerror(l, "Failed to allocate new object");
    }
    memset(o, 0, sizeof(*o));
    o->is3d = is3d;

    // add us to the object list:
    o->next = objects;
    if (objects) {
        objects->prev = o;
    }
    objects = o;

    // if resource is present, start loading it:
    luafuncs_objectgraphics_load(o, resource);
    return 0;
}

// implicitely calls luafuncs_onError() when an error happened
void luacfuncs_object_callEvent(struct blitwizardobject* o,
const char* eventName) {

}

/// Delete the given object explicitely, to make it instantly disappear
/// from the game world.
// @function delete
int luafuncs_object_delete(lua_State* l) {
    // delete the given object
    struct blitwizardobject* o = toblitwizardobject(l, 1, 1, "blitwiz.object.delete");
    if (o->deleted) {
        lua_pushstring(l, "Object was deleted");
        return lua_error(l);
    }

    // mark it deleted, and move it over to deletedobjects:
    o->deleted = 1;
    if (o->prev) {
      o->prev->next = o->next;
    } else {
      objects = o->next;
    }
    if (o->next) {
      o->next->prev = o->prev;
    }
    o->next = deletedobjects;
    deletedobjects->prev = o;
    deletedobjects = o;
    o->prev = NULL;

    // destroy the drawing and physics things attached to it:
    cleanupobject(o);
    return 0;
}

/// Get the current position of the given object.
// Returns two coordinates for a 2d object, and three coordinates
// for a 3d object.
// @function getPosition
// @treturn number x coordinate
// @treturn number y coordinate
// @treturn number (if 3d object) z coordinate
int luafuncs_getPosition(lua_State* l) {
    struct blitwizardobject* obj = toblitwizardobject(l, 1, 0,
    "blitwizard.object:getPosition");
    if (obj->deleted) {
        return haveluaerror(l, "Object was deleted");
    }
    double x,y,z;
    objectphysics_getPosition(obj, &x, &y, &z);
    lua_pushnumber(l, x);
    lua_pushnumber(l, y);
    if (obj->is3d) {
        lua_pushnumber(l, z);
        return 3;
    }
    return 2;
}

