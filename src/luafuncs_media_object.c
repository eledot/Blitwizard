
/* blitwizard game engine - source code file

  Copyright (C) 2012-2013 Jonas Thiem

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

/// The blitwizard audio namespace allows you to create
// sound objects to play sounds and music.
// Use @{blitwizard.audio.simpleSound|simpleSound} objects for
// background music, @{blitwizard.audio.pannedSound|blitwizard.audio.pannedSound}
// for a sound with left/right panning,
// @{blitwizard.audio.positionedSound|blitwizard.audio.positionedSound} for a sound with
// positioning in 2d or 3d space inside the game world.
// @author Jonas Thiem  (jonas.thiem@gmail.com)
// @copyright 2012-2013
// @license zlib
// @module blitwizard.audio

#include "luafuncs_media_object.h"

int luafuncs_media_object_new(lua_State* l, int type) {

}

int luafuncs_media_object_destroy(struct mediaobject* o, int type) {

}

// @type simpleSound

/// Create a new simple sound object which has no
// stereo left/right panning or room positioning features.
// This is the sound object suited best for background music.
// It deactivates some postprocessing which allows positioning
// or stereo panning which results in slightly better and
// unaltered sound.
// @function new
int luafuncs_media_simpleSound_new(lua_State* l) {
    return luafuncs_media_object_new(l, MEDIA_TYPE_AUDIO_SIMPLE);
}

// @module blitwizard.audio
// @type pannedSound

// @module blitwizard.audio
// @type positionSound



// Various cleanup and management functions:

void cleanupmediaobject(struct blitwizardobject* o) {
    
}

static int garbagecollect_mediaobjref(lua_State* l) {
    // we need to decrease our reference count of the
    // media object we referenced to.

    // get id reference to object
    struct luaidref* idref = lua_touserdata(l, -1);

    if (!idref || idref->magic != IDREF_MAGIC
    || idref->type != IDREF_MEDIA) {
        // either wrong magic (-> not a luaidref) or not a media object
        lua_pushstring(l, "internal error: invalid media object ref");
        lua_error(l);
        return 0;
    }

    // it is a valid media object, decrease ref count:
    struct mediaobject* o = idref->ref.bobj;
    o->refcount--;

    // if it's already deleted and ref count is zero, remove it
    // entirely and free it:
    if (o->deleted && o->refcount <= 0) {
        cleanupmediaobject(o);

        // remove object from the list
        if (o->prev) {
            // adjust prev object to have new next pointer
            o->prev->next = o->next;
        } else {
            // was first object in deleted list -> adjust list start pointer
            deletedMediaObjects = o->next;
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

