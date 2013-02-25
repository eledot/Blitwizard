
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

#include "os.h"
#include <stdint.h>
#ifdef HAVE_SDL
#include "SDL.h"
#else
#ifdef WINDOWS
#include <windows.h>
#else
#include <time.h>
#include <unistd.h>
#include <string.h>
#endif
#endif
#include "timefuncs.h"
#ifdef MAC
#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

#if (!defined(HAVE_SDL) && defined(UNIX))
#ifdef MAC
uint64_t lastunixtime;
#else
static struct timespec lastunixtime;
uint64_t lasttimestamp = 0;
#endif
int startinitialised = 0;
#endif

uint64_t oldtime = 0;
uint64_t timeoffset = 0;
uint64_t time_GetMilliseconds() {
#if defined(HAVE_SDL) || defined(WINDOWS)
#ifdef HAVE_SDL
    uint64_t i = SDL_GetTicks();
#else
    uint64_t i = GetTickCount();
#endif
    i += timeoffset; // add an offset we might have set
    if (i > oldtime) {
        // normal time difference
        oldtime = i;
    }else{
        // we wrapped around. set a time offset to avoid the wrap
        timeoffset = (oldtime - i) + 1;
        i += timeoffset;
    }
#else // ifdef HAVE_SDL
#ifdef MAC
    // MAC NO-SDL TIME
    if (!startinitialised) {
        lastunixtime = mach_absolute_time();
        startinitialised = 1;
        return 0;
    }
    uint64_t elapsed = mach_absolute_time() - lastunixtime;
    Nanoseconds ns = AbsoluteToNanoseconds(*(AbsoluteTime*)&elapsed);
    return *(uint64_t)&ns;
#else // ifdef MAC
    // UNIX/LINUX NO-SDL TIME
    if (!startinitialised) {
        clock_gettime(CLOCK_MONOTONIC, &lastunixtime);
        startinitialised = 1;
        lasttimestamp = 0;
        return 0;
    }
    struct timespec current;
    clock_gettime(CLOCK_MONOTONIC, &current);
    int64_t seconds = current.tv_sec - lastunixtime.tv_sec;
    int64_t nseconds = current.tv_nsec - lastunixtime.tv_nsec;
    uint64_t i = (uint64_t)((double)((seconds) * 1000 + nseconds / 1000000.0) + 0.5);
    // to avoid accuracy issues in our continuously increased timestamp due
    // to all the rounding business happening above, we only update our old
    // reference check time (lastunixtime/lasttimestamp) once a full second
    // has passed:
    if (i > 1000) {
        i += lasttimestamp;
        lasttimestamp = i;
        memcpy(&lastunixtime, &current, sizeof(struct timespec));
    }else{
        i += lasttimestamp;
    }
#endif // ifdef MAC
#endif // ifdef HAVE_SDL
    return i;
}

void time_Sleep(uint32_t milliseconds) {
#ifdef HAVE_SDL
    SDL_Delay(milliseconds);
#else
#ifdef WINDOWS
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
#endif
}

