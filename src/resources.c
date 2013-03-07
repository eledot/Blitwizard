
/* blitwizard game engine - source code file

  Copyright (C) 2013 Jonas Thiem

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

#include <string.h>
#include <stdio.h>
#include "resources.h"
#include "file.h"

int resources_LoadZip(const char* path) {
#ifdef USE_PHYSFS
    return 0;  // unimplemented for now
#else  // USE_PHYSFS
    // no PhysFS -> no .zip support
    return 0;
#endif  // USE_PHYSFS
}

int resource_LoadZipFromExecutable(const char* path) {
#ifdef USE_PHYSFS
#ifdef WINDOWS
    // on windows and using PhysFS

#else  // WINDOWS
    // on some unix with PhysFS
    return 0;
#endif  // WINDOWS
#else  // USE_PHYSFS
    // no PhysFS -> no .zip support
    return 0;
#endif  // USE_PHYSFS
}

int resource_LoadZipFromOwnExecutable(const char* first_commandline_arg) {
#ifdef WINDOWS

#else
    // locate our own binary using the first command line arg
    if (!first_commandline_arg) {
        // no way of locating it.
        return 0;
    }
    printf("first argument: %s\n",
    first_commandline_arg);
    return 0;
#endif
}


int resource_LocateResource(const char* path,
struct resourcelocation* location) {
#ifdef USE_PHYSFS
    // check resource archives:

#endif
    // check the hard disk as last location:
    if (file_DoesFileExist(path) && !file_IsDirectory(path)) {
        if (location) {
            location->type = LOCATION_TYPE_DISK;
            int i = strlen(path);
            if (i >= MAX_RESOURCE_PATH) {
                i = MAX_RESOURCE_PATH-1;
            }
            memcpy(location->location.disklocation.filepath, path, i);
            location->location.disklocation.filepath[i] = 0;
        }
        return 1;
    }
    return 0;
}


