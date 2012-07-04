
/* blitwizard 2d engine - source code file

  Copyright (C) 2012 Jonas Thiem

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

#ifdef __cplusplus
extern "C" {
#endif

double getangle(double x, double y, double x2, double y2);
double getdist(double x, double y, double x2, double y2);
void pointonline(double x1, double y1, double x2, double y2, double px, double py, double* linepointx, double* linepointy, double* relativepos);
void rotatevec(double x, double y, double rotation, double* x2, double* y2);
double normalizeangle(double angle);
void ovalpoint(double angle, double width, double height, double* x, double* y);

#ifndef NOLLIMITS
#define FASTMATH
#endif

#ifdef FASTMATH

// use the fast double > int32 conversion from Lua
#include "lua.h"
#include "llimits.h"
static inline int fastdoubletoint32(double i) {
    int n;
    lua_number2int(n, i);
    return n;
}

#else

// use slow normal cast
static inline int fastdoubletoint32(double i) {
    return (int)i;
}

#endif

#ifdef __cplusplus
}
#endif

