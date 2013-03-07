
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
#include <stdlib.h>
#include <stdio.h>
#include "os.h"

#ifdef USE_PHYSFS

#include "file.h"
#include "physfs.h"
#include "zipfile.h"

#define MOUNT_POINT_LEN 20

struct zipfile {
    struct PHYSFS_Io* physio;
    FILE* f;  // file handle
    char* fname;  // file name, required for duplication
    size_t offsetinfile;
    size_t sizeinfile;
    size_t posinfile;
    char mountpoint[MOUNT_POINT_LEN+1];
};
struct PHYSFS_Io* physio = NULL;

__attribute__((constructor)) void initialisezip(void) {
    // initialize physfs and just pass some random stuff for binary name:
    PHYSFS_init("./ourrandomnonexistingbinaryname");
}

static PHYSFS_sint64 zipfile_Read(struct PHYSFS_Io *io, void *buf,
PHYSFS_uint64 len) {
    // get our zip file info:
    struct zipfile* zf = (struct zipfile*)(io->opaque);

    // simply read from our file handle, but no further than the limit:
    size_t readlen = len;
    if (readlen > zf->sizeinfile - (zf->posinfile + zf->offsetinfile)) {
        readlen = zf->sizeinfile - (zf->posinfile + zf->offsetinfile);
    }
    if (readlen == 0) {
        return 0;
    }
    size_t returnvalue = fread(buf, 1, readlen, zf->f);
    zf->posinfile += returnvalue;
    return returnvalue;
}

static PHYSFS_sint64 zipfile_Tell(struct PHYSFS_Io *io) {
    // get our zip file info:
    struct zipfile* zf = (struct zipfile*)io->opaque;

    // return position:
    return zf->posinfile;
}

static PHYSFS_sint64 zipfile_Length(struct PHYSFS_Io *io) {
    // get our zip file info:
    struct zipfile* zf = (struct zipfile*)io->opaque;

    // return length:
    return zf->sizeinfile;
}

static PHYSFS_sint64 zipfile_Seek(struct PHYSFS_Io *io,
PHYSFS_uint64 offset) {
    // get our zip file info:
    struct zipfile* zf = (struct zipfile*)io->opaque;

    if (offset > zf->sizeinfile) {
        return 0;  // out of bounds
    }

    // attempt to seek:
    int i = fseek(zf->f, offset, SEEK_SET);
    if (i == 0) {
        // success
        zf->posinfile = offset;
        return 1;
    }
    // error
    return 0;
}

static void zipfile_Destroy(struct PHYSFS_Io *io) {
    // close file, destroy our structs:
    struct zipfile* zf = (struct zipfile*)(io->opaque);
    fclose(zf->f);
    free(zf->fname);
    free(zf->physio);
}

struct PHYSFS_Io* zipfile_Duplicate(struct PHYSFS_Io* io) {
    // duplicate zipfile struct:
    struct zipfile* zf = (struct zipfile*)(io->opaque);
    struct zipfile* newzip = malloc(sizeof(*newzip));
    if (!newzip) {
        return NULL;
    }
    memcpy(newzip, zf, sizeof(*newzip));

    // duplicate physio struct:
    newzip->physio = malloc(sizeof(*(newzip->physio)));
    if (!newzip->physio) {
        free(newzip);
        return NULL;
    }
    memcpy(newzip->physio, zf->physio, sizeof(*(newzip->physio)));
    newzip->physio->opaque = newzip;

    // duplicate file name:
    newzip->fname = strdup(zf->fname);
    if (!newzip->fname) {
        free(newzip->physio);
        free(newzip);
        return NULL;
    }

    // "duplicate" file handle by simply reopening the file:
    newzip->f = fopen(newzip->fname, "rb");
    if (!newzip->f) {
        free(newzip->fname);
        free(newzip->physio);
        free(newzip);
        return NULL;
    }
    return newzip->physio;
}

static void zipfile_RandomMountPoint(char* buffer) {
    buffer[0] = '/';
    int i = 1;
    while (i < MOUNT_POINT_LEN-1) {
#ifdef WINDOWS
        double d = (double)((double)rand())/RAND_MAX;
#else
        double d = drand48();
#endif
        int i = (int)((double)d*9.99);
        buffer[i] = '0' + i;
        i++;
    }
    buffer[MOUNT_POINT_LEN-1] = '/';
    buffer[MOUNT_POINT_LEN] = 0;
}

struct zipfile* zipfile_Open(const char* file, size_t offsetinfile,
size_t sizeinfile) {
    if (!file || strlen(file) <= 0) {
        // no file given. this is not valid
        return NULL;
    }

    // allocate our basic zip info struct:
    struct zipfile* zf = malloc(sizeof(*zf));
    if (!zf) {
        return NULL;
    }
    memset(zf, 0, sizeof(*zf));
    zf->offsetinfile = offsetinfile;
    zf->sizeinfile = sizeinfile;

    // prepare file name:
    zf->fname = strdup(file);
    if (!zf->fname) {
        free(zf);
        return NULL;
    }
    file_MakeSlashesNative(zf->fname);

    // open file name:
    zf->f = fopen(zf->fname, "rb");
    if (!zf->f) {
        free(zf->fname);
        free(zf);
        return NULL;
    }

    // initialise sizeinfile to something senseful if zero:
    if (zf->sizeinfile == 0) {
        // initialise to possible file size:
        zf->sizeinfile = file_GetSize(zf->fname)-zf->offsetinfile;
    }

    // allocate physfs io struct:
    zf->physio = malloc(sizeof(*physio));
    if (!zf->physio) {
        fclose(zf->f);
        free(zf->fname);
        free(zf);
        return NULL;
    }

    memset(zf->physio, 0, sizeof(*(zf->physio)));
    zf->physio->opaque = zf;  // make sure we can find our zip info again

    // initialise our custom file access functions:
    zf->physio->read = &zipfile_Read;
    zf->physio->length = &zipfile_Length;
    zf->physio->tell = &zipfile_Tell;
    zf->physio->seek = &zipfile_Seek;
    zf->physio->duplicate = &zipfile_Duplicate;
    zf->physio->destroy = &zipfile_Destroy;

    // mount the file to a random mount point:
    zipfile_RandomMountPoint(zf->mountpoint);
    if (!PHYSFS_mountIo(zf->physio, NULL, zf->mountpoint, 1)) {
        zf->physio->destroy(zf->physio);
        return 0;
    }
    return 1;
}

void zipfile_Close(struct zipfile* zf) {
    PHYSFS_unmount(zf->mountpoint);
}

#endif  // USE_PHYSFS

