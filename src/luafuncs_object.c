
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

#include "graphics.h"
#include "luafuncs_object.h"

struct blitwizardobject {
    int 3d;  // 0: 2d sprite with z-order value, 1: 3d mesh or sprite
    double x,y;  // 2d: x,y, 3d: x,y,z with z pointing up
    int deleted;  // 1: deleted (deletedobjects), 0: regular (objects)
    int refcount;  // refcount of luaidref references
    union {
        double z;
        int zindex;
    } vpos;
    struct blitwizardobject* prev,*next;
};

struct blitwizardobject* objects = NULL;
struct blitwizardobject* deletedobjects = NULL;

void luafuncs_pushbobjidref(lua_State* l, struct blitwizardobject* o) {
    // create luaidref userdata struct which points to the blitwizard object
    struct luaidref* ref = lua_newuserdata(l, sizeof(*ref));
    memset(ref, 0, sizeof(*ref));
    ref->magic = IDREF_MAGIC;
    ref->type = IDREF_BLITWIZARDOBJECT;
    ref->ref.bobj = o;
}

static int garbagecollect_blitwizobj(lua_State* l) {
    // we need to decrease our reference count of the
    // blitwizard object we referenced to.

    // get id reference to object
    struct luaidref* idref = lua_touserdata(l, -1);

    if (!idref || idref->magic != IDREF_MAGIC
    || idref->type != IDREF_BLITWIZARDOBJECT) {
        // either wrong magic (-> not a luaidref) or not a blitwizard object
        lua_pushstring(l, invalid);
        lua_error(l);
        return NULL;
    }

    // it is a valid blitwizard object, decrease ref count:
    struct blitwizardobject* o = idref->ref.bobj;
    ((struct blitwizardobject*)idref->ref.ptr)->refcount--;

    // if it's already deleted and ref count is zero, remove it
    // entirely and free it:
    if (o->deleted && o->refcount <= 0) {
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

int luafuncs_object_create2d() {
    // create new object on the two dimensional layer
    struct blitwizardobject* o = malloc(sizeof(*o));
    if (!o) {
        return haveluaerror(l, "Failed to allocate new object");
    }
    memset(o, 0, sizeof(*o));
    o->3d = 0;    

    luafuncs_pushbobjidref(l, o);
    luastate_SetGCCallback(l, -1, (int (*)(void*))&garbagecollect_blitwizobj);
    return 1;
}

int luafuncs_object_create3d() {
    // create new object in the 3d space
    struct blitwizardobject* o = malloc(sizeof(*o));
    if (!o) {
        return haveluaerror(l, "Failed to allocate new object");
    }
    memset(o, 0, sizeof(*o));
    o->3d = 1;

    luafuncs_pushbobjidref(l, o);
    luastate_SetGCCallback(l, -1, (int (*)(void*))&garbagecollect_blitwizobj);
    return 1;
}

