
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

#ifndef BLITWIZARD_MEDIAOBJECT_H_
#define BLITWIZARD_MEDIAOBJECT_H_

#include "os.h"

#define MEDIA_TYPE_AUDIO_SIMPLE 1
#define MEDIA_TYPE_AUDIO_PANNED 2
#define MEDIA_TYPE_AUDIO_POSITIONED 3

struct mediaobject {
    int type;
    int isPlaying;
    int refcount;  // refcount of luaidref references
    union {
        struct {
            int priority;
            float volume;
            float panning;
            int is3d;
            double x,y,z;  // 2d: x,y, 3d: x,y,z with z pointing up
            int soundid;
            const char* soundname;
        } sound;
    } mediainfo;
    struct mediaobject* prev,*next;
};

extern struct mediaobject* mediaObjects;

#endif  // BLITWIZARD_MEDIAOBJECT_H_

