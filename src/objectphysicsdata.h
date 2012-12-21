
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

*/

#ifndef BLITWIZARD_OBJECTPHYSICSDATA_H_
#define BLITWIZARD_OBJECTPHYSICSDATA_H_

#if (defined(USE_PHYSICS2D) || defined(USE_PHYSICS3D))

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "physics.h"

struct objectphysicsdata {
    int refcount;
    int movable;
    struct physicsobject* object;
    double friction;
    double restitution;
    double lineardamping;
    double angulardamping;
    int rotationrestriction;
    int deleted;
};

#endif  // USE_PHYSICS2D || USE_PHYSICS3D

#endif  // BLITWIZARD_OBJECTPHYSICSDATA_H_

