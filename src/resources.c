
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

int resources_LoadZipFromFilePart(const char* path,
size_t offsetinfile, size_t sizeinfile) {
    // unimplemented for now
    return 0;
}

int resources_LoadZipFromFile(const char* path) {
#ifdef USE_PHYSFS
    return resources_LoadZipFromFilePart(path, 0, 0);
#else  // USE_PHYSFS
    // no PhysFS -> no .zip support
    return 0;
#endif  // USE_PHYSFS
}

int resource_LoadZipFromExecutable(const char* path) {
#ifdef USE_PHYSFS
#ifdef WINDOWS
    // on windows and using PhysFS
    // open file:
    FILE* r = fopen(path, "rb");
    if (!r) {
        return 0;
    }

    // read DOS header:
    IMAGE_DOS_HEADER h;
    int k = fread(&h, 1, sizeof(h), r);
    if (k != sizeof(h)) {
        fclose(r);
    }

    // locate PE header
    IMAGE_NT_HEADERS32 nth;
    fseek(r, h.e_lfanew, SEEK_SET);
    if (fread(&nth, 1, sizeof(nth), r) != sizeof(nth)) {
        fclose(r);
    }

    // check header signatures:
    if (dosheader.e_magic != IMAGE_DOS_SIGNATURE ||
    header->Signature != IMAGE_NT_SIGNATURE) {
        fclose(r);
        return 0;
    }

    // find the section that has the highest offset in the file
    // (= latest section)
    IMAGE_SECTION_HEADER sections;
    if (fread(&sections, 1, sizeof(sections), r) != sizeof(sections)) {
        fclose(r);
        return 0;
    }
    fclose(r);
    void* latestSection = NULL;
    int latestSectionIndex = -1;
    int i = 0;
    while (i < nth.fileHeader.NumberOfSections) {
        if (sections[i].PointerToRawData > latestSection) {
            latestSection = sections[i].PointerToRawData;
            latestSectionIndex = i;
        }
        i++;
    }

    if (!latestSection) {
        // no latest section. weird!
        return 0;
    }

    // the size of the PE section data:
    size_t size = (size_t)sections[i].PointerToRawData +
    sections[i].SizeOfRawData;

    // now we have all information we want.
    return resource_LoadZipFromFilePart(path, size, 0);
#else  // WINDOWS
    // on some unix with PhysFS
    // open file:
    FILE* r = fopen(path, "rb");

    // see if this is an ELF file:
    char magic[4];
    if (fread(magic, 1, 4, r) != 4) {
        fclose(r);
        return 0;
    }
    if (magic[0] != 127 || memcmp("ELF", magic+1, 3) != 0) {
        // not an ELF binary
        fclose(r);
        return 0;
    }
    printf("ELF binary is at: %s\n", path);

    // read regular size of ELF binary from header:


    // close file:
    fclose(r);
    return 0;
#endif  // WINDOWS
#else  // USE_PHYSFS
    // no PhysFS -> no .zip support
    return 0;
#endif  // USE_PHYSFS
}

int resource_LoadZipFromOwnExecutable(const char* first_commandline_arg) {
#ifdef WINDOWS
    // locate our exe:
    char fname[512];
    if (!GetModuleFileName(NULL, fname, sizeof(fname))) {
        return 0;
    }

    // get an absolute path just in case it isn't one:
    char* path = file_GetAbsolutePathFromRelativePath(fname);
    if (!path) {
        return 0;
    }

    // load from our executable:
    int result = resource_LoadZipFromExecutable(fname);
    free(path);
    return result;
#else
    // locate our own binary using the first command line arg
    if (!first_commandline_arg) {
        // no way of locating it.
        return 0;
    }

    // check if the given argument is a valid file:
    if (!file_DoesFileExist(first_commandline_arg)
    || file_IsDirectory(first_commandline_arg)) {
        // not pointing at a file as expected
        return 0;
    }

    // get an absolute path:
    char* path = file_GetAbsolutePathFromRelativePath(first_commandline_arg);
    if (!path) {
        return 0;
    }

    // load from our executable:
    int result = resource_LoadZipFromExecutable(path);
    free(path);
    return result;
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


