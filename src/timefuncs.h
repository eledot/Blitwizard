
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

#ifndef BLITWIZARD_TIMEFUNCS_H_
#define BLITWIZARD_TIMEFUNCS_H_

uint64_t time_GetMilliseconds(void);
// Timestamp derived from SDL_GetTicks(),
// but deals with wraps (SDL_GetTicks() is originally
// an uint32_t).
//
// When a wrap happens, it is detected and the time is
// forced to proceed from the last returned value + 1.
//
// This results in an inaccurate time returned when
//  a wrap happens relative to the previous time stamp
// returned, but the time will never wrap/suddenly
// jump backwards until uint64_t is exceeded.

void time_Sleep(uint32_t milliseconds);
// Sleep for a specified amount of time.

#endif  // BLITWIZARD_TIMEFUNCS_H_

