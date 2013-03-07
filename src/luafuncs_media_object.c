
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
// background music, @{blitwizard.audio.pannedSound|pannedSound}
// for a sound with left/right panning and
// @{blitwizard.audio.positionedSound|positionedSound} for a sound with
// positioning in 2d or 3d space inside the game world.
//
// Please note blitwizard will NEVER preload any sounds into memory,
// so it is perfectly fine to create many sound objects from the same
// sound file, it won't cause the file to be loaded into memory
// many times.
//
// Supported audio formats are .ogg and .flac, and, if a usable FFmpeg
// version is present on the system, many more like .mp3, .mp4 and others.
//
// @author Jonas Thiem  (jonas.thiem@gmail.com)
// @copyright 2012-2013
// @license zlib
// @module blitwizard.audio

#include "luafuncs_media_object.h"
#include "luaheader.h"
#include "luastate.h"

#include <stdlib.h>
#include <string.h>

int luafuncs_media_object_new(lua_State* l, int type) {

}

int luafuncs_media_object_play(lua_State* l, int type) {

}

int luafuncs_media_object_stop(lua_State* l, int type) {

}

/// Implements a simple sound which has no
// stereo left/right panning or room positioning features.
// This is the sound object suited best for background music.
// It deactivates some of the postprocessing which would allow positioning
// or stereo panning which results in slightly better and
// unaltered sound.
// 
// The default @{blitwizard.audio.simpleSound:setPriority|sound priority}
// for simple sound objects is 5.
//
// @type simpleSound

/// Create a new simple sound object.
// @function new
// @tparam string filename Filename of the audio file you want to play
// @usage
// mysound = blitwizard.audio.simpleSound:new("blubber.ogg")
// mysound:play()

int luafuncs_media_simpleSound_new(lua_State* l) {
    return luafuncs_media_object_new(l, MEDIA_TYPE_AUDIO_SIMPLE);
}

/// Play the sound represented by the simple sound object
// @function play
// @tparam number volume (optional) Volume at which the sound plays from 0 (quiet) to 1 (full volume). Defaults to 1
// @tparam boolean loop (optional) If set to true, the sound will loop until explicitely stopped. If set to false or if not specified, it will play once
// @tparam number fadein (optional) Fade in from silence to the specified volume in the given amount of seconds, instead of playing at full volume right from the start

int luafuncs_media_simpleSound_play(lua_State* l) {
    return luafuncs_media_object_play(l, MEDIA_TYPE_AUDIO_SIMPLE);
}

/// Stop the sound represented by the simple sound object.
// Does nothing if the sound doesn't currently play
// @function stop
// @tparam number fadeout (optional) If specified, the sound will fade out for the specified amount in seconds. Otherwise it will stop instantly

int luafuncs_media_simpleSound_stop(lua_State* l) {
    return luafuncs_media_object_stop(l, MEDIA_TYPE_AUDIO_SIMPLE);
}

/// Set the sound priority of the simple sound object.
//
// In blitwizard, only a fixed maximum number of sounds can play
// at a time. If you attempt to @{blitwizard.audio.simpleSound:play|play}
// a sound object when the maximum number of sounds simultaneously
// is reached, it will kill off playing sounds with
// lower priority. If no lower or same priority sound is available
// to kill, it won't start playing at all.
//
// Changing the priority won't affect the sound object's current
// playing, it will only have an effect the next time you use
// play.
//
// Background music should have a high priority, and short,
// frequent sounds like gun shots or footsteps should have a low
// priority.
//
// Simple sound objects default to a priority of 5.
// @function setPriority
// @tparam number priority Priority from 0 (lowest) to 20 (highest), values will be rounded down to have no decimal places (0.5 becomes 0, 1.7 becomes 1, etc)

int luafuncs_media_simpleSound_setPriority(lua_State* l) {
    return luafuncs_media_object_setPriority(l, MEDIA_TYPE_AUDIO_SIMPLE);
}

/// Adjust the volume of a simple sound while it is playing
// (does nothing if it's not)
// @function adjust
// @tparam number volume New volume from 0 (quiet) to 1 (full volume)

int luafuncs_media_simpleSound_adjust(lua_State* l) {
    return luafuncs_media_object_adjust(l, MEDIA_TYPE_AUDIO_SIMPLE);
}

/// Implements a sound with simple left/right stereo panning.
// If you want to make a sound emit from a specific location,
// you should probably use a
// @{blitwizard.audio.positionedSound|positionedSound} instead
// @type pannedSound

/// Create a new panned sound object.
// @function new
// @tparam string filename Filename of the audio file you want to play

int luafuncs_media_pannedSound_new(lua_State* l) {
    return luafuncs_media_object_new(l, MEDIA_TYPE_AUDIO_PANNED);
}

/// Play the sound represented by the panned sound object
// @function play
// @tparam number volume (optional) Volume at which the sound plays from 0 (quiet) to 1 (full volume). Defaults to 1
// @tparam number panning (optional) Stereo panning which alters the left/right placement of the sound from 1 (left) through 0 (center) to -1 (right). Default is 0
// @tparam boolean loop (optional) If set to true, the sound will loop until explicitely stopped. If set to false or if not specified, it will play once
// @tparam number fadein (optional) Fade in from silence to the specified volume in the given amount of seconds, instead of playing at full volume right from the start

int luafuncs_media_pannedSound_play(lua_State* l) {
    return luafuncs_media_object_play(l, MEDIA_TYPE_AUDIO_PANNED);
}

/// Stop the sound represented by the panned sound object.
// Does nothing if the sound doesn't currently play
// @function stop
// @tparam number fadeout (optional) If specified, the sound will fade out for the specified amount in seconds. Otherwise it will stop instantly

int luafuncs_media_pannedSound_stop(lua_State* l) {
    return luafuncs_media_object_stop(l, MEDIA_TYPE_AUDIO_PANNED);
}

/// Set the sound priority of the panned sound,
// see explanation of @{blitwizard.audio.simpleSound:setPriority}.
//
// Panned sounds default to a priority of 2.
// @function setPriority
// @tparam number priority Priority from 0 (lowest) to 20 (highest), values will be rounded down to have no decimal places (0.5 becomes 0, 1.7 becomes 1, etc)

int luafuncs_media_pannedSound_setPriority(lua_State* l) {
    return luafuncs_media_object_setPriority(l, MEDIA_TYPE_AUDIO_SIMPLE);
}

/// Implements a positioned sound which can either follow a
// specific @{blitwizard.object|blitwizard object} or which
// you can move to any position you like. It will alter volume
// based on distance and channels based on direction to simulate
// its room placement.
// @type positionedSound


/// Create a new positioned sound object.
// @function new
// @tparam string filename Filename of the audio file you want to play
// @tparam userdata object (optional) Specify a @{blitwizard.object|blitwizard object} to which the sound should stick. If you don't specify an object to stick to, the sound can be freely placed with @{position}

int luafuncs_media_positionedSound_new(lua_State* l) {
    return luafuncs_media_object_new(l, MEDIA_TYPE_AUDIO_POSITIONED);
}


/// Stop the sound represented by the positioned sound object.
// Does nothing if the sound doesn't currently play
// @function stop
// @tparam number fadeout (optional) If specified, the sound will fade out for the specified amount in seconds. Otherwise it will stop instantly

int luafuncs_media_positionedSound_stop(lua_State* l) {
    return luafuncs_media_object_stop(l, MEDIA_TYPE_AUDIO_POSITIONED);
}



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

    // if it's not playing and ref count is zero, remove it
    // entirely and free it:
    if (!o->isPlaying && o->refcount <= 0) {
        cleanupmediaobject(o);

        // remove object from the list
        if (o->prev) {
            // adjust prev object to have new next pointer
            o->prev->next = o->next;
        } else {
            // was first object in list -> adjust list start pointer
            mediaObjects = o->next;
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

