
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

#ifndef BLITWIZARD_RESOURCES_H_
#define BLITWIZARD_RESOURCES_H_

#include <stdlib.h>

// The resources code manages all loaded resource zip files
// and decides where to pick any resource from (image, sound, ...).

// Manage resource zip files:
int resources_LoadZipFromFile(const char* path, int encrypted);
int resources_LoadZipFromFilePart(const char* path,
size_t offsetinfile, size_t sizeinfile, int encrypted);
int resources_LoadZipFromExecutable(const char* path,
int encrypted);
int resources_LoadZipFromOwnExecutable(
const char* first_commandline_arg, int encrypted);
// (all functions return 1 on success, 0 on error)

// Resource location info:
#define LOCATION_TYPE_ZIP 1
#define LOCATION_TYPE_DISK 2
#define MAX_RESOURCE_PATH 100
struct resourcelocation {
    int type;  // the location type
    union {
        // specified for LOCATION_TYPE_ZIP
        // (= resource is inside a zip archive)
        struct {
            // zip archive the resource is in:
            struct zipfile* archive;
            // file path of the file inside the archive:
            char filepath[MAX_RESOURCE_PATH];
        } ziplocation;
        // specified for LOCATION_TYPE_DISK:
        // (= resource is at a regular hard disk location)
        struct {
            // absolute file path to file on disk:
            char filepath[MAX_RESOURCE_PATH];
        } disklocation;
    } location;
};

// Locate a resource:
int resources_LocateResource(const char* resource,
struct resourcelocation* location);
// Returns 1 if resource is found, in which case the given
// resourceinfo struct is modified to share the information.
// Returns 0 if the resource wasn't found.

#endif  // BLITWIZARD_RESOURCES_H_

