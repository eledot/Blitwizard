
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
#include "luafuncs_object_media.h"

struct blitwizardobject* objects = NULL;
struct blitwizardobject* deletedobjects = NULL;

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

static struct blitwizardobject* toblitwizardobject(lua_State* l, int index, int arg, const char* func) {
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

int luafuncs_object_create2d(lua_State* l) {
    // create new object on the two dimensional layer
    struct blitwizardobject* o = malloc(sizeof(*o));
    if (!o) {
        return haveluaerror(l, "Failed to allocate new object");
    }
    memset(o, 0, sizeof(*o));
    o->is3d = 0;    
    luacfuncs_object_media_load(o);
    return 1;
}

int luafuncs_object_create3d(lua_State* l) {
    // create new object in the 3d space
    struct blitwizardobject* o = malloc(sizeof(*o));
    if (!o) {
        return haveluaerror(l, "Failed to allocate new object");
    }
    memset(o, 0, sizeof(*o));
    o->is3d = 1;
    luacfuncs_object_media_load(o);
    return 1;
}

int luafuncs_object_delete(lua_State* l) {
    // delete the given object
    struct blitwizardobject* o = toblitwizardobject(l, 1, 1, "blitwiz.object.delete");

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
    deletedobjects = o;
    o->prev = NULL;

    // destroy the drawing and physics things attached to it:
    luacfuncs_object_media_unload(o);
    return 0;
}
