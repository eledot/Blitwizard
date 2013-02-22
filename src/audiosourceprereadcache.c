
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

#include <string.h>
#include <stdlib.h>

#include "os.h"
#include "audiosource.h"
#include "audiosourceprereadcache.h"

struct audiosourceprereadcache_internaldata {
    struct audiosource* source;
    unsigned int prereadcachesize;
    char* prereadcache;
    unsigned int prereadcachebytes;
    int eof;
    int sourceeof;
    char* path;
};

static void audiosourceprereadcache_Rewind(struct audiosource* source) {
    struct audiosourceprereadcache_internaldata* idata = source->internaldata;
    idata->eof = 0;
    idata->source->rewind(idata->source);
    idata->sourceeof = 0;
    idata->prereadcachebytes = 0;
}

static int audiosourceprereadcache_Read(struct audiosource* source, char* buffer, unsigned int bytes) {
    struct audiosourceprereadcache_internaldata* idata = source->internaldata;
    if (idata->eof) {
        return -1;
    }
    if (idata->prereadcachesize == 0) {
        // cache was not initialised yet - initialise with standard size
        idata->prereadcachesize = (1024 * 10);
        idata->prereadcache = malloc(idata->prereadcachesize);
        if (!idata->prereadcache) {
            return -1;
        }
    }
    if (!idata->prereadcache) {
        return -1;
    }

    unsigned int writtenbytes = 0;
    while (bytes > 0) {
        // first, refill our cache if it cannot satisfy the demands
        if (bytes > idata->prereadcachebytes/2 && (idata->prereadcachebytes < idata->prereadcachesize/4
            || bytes > idata->prereadcachebytes) && !idata->sourceeof) {
            int i;
            i = idata->source->read(idata->source, idata->prereadcache + idata->prereadcachebytes, idata->prereadcachesize - idata->prereadcachebytes);
            if (i > 0) {
                idata->prereadcachebytes += i;
            }else{
                idata->sourceeof = 1;
                if (i < 0) {
                    idata->eof = 1;
                    return -1;
                }
            }
        }

        if (bytes > 0) {
            if (idata->prereadcachebytes > 0) {
                // we have some cached bytes - return them
                unsigned int readbytes = idata->prereadcachebytes;
                if (readbytes > bytes) {
                    readbytes = bytes;
                }
                memcpy(buffer, idata->prereadcache, readbytes);
                buffer += readbytes;
                bytes -= readbytes;
                writtenbytes += readbytes;

                // trim data in cache buffer
                if (readbytes < idata->prereadcachebytes) {
                    memmove(idata->prereadcache, idata->prereadcache + readbytes, idata->prereadcachebytes - readbytes);
                }
                idata->prereadcachebytes -= readbytes;
            }else{
                // we were not able to get new cached bytes -> End of Stream
                if (writtenbytes == 0) {
                    idata->eof = 1;
                }
                return writtenbytes;
            }
        }
    }
    return writtenbytes;
}

static void audiosourceprereadcache_Close(struct audiosource* source) {
    struct audiosourceprereadcache_internaldata* idata = source->internaldata;
    if (idata->source) {
        idata->source->close(idata->source);
    }
    if (idata->prereadcache) {
        free(idata->prereadcache);
    }
    free(idata);
    free(source);
}

struct audiosource* audiosourceprereadcache_Create(struct audiosource* source) {
    if (!source) {
        return NULL;
    }
    struct audiosource* a = malloc(sizeof(*a));
    if (!a) {
        return NULL;
    }

    memset(a,0,sizeof(*a));
    a->internaldata = malloc(sizeof(struct audiosourceprereadcache_internaldata));
    if (!a->internaldata) {
        free(a);
        return NULL;
    }

    struct audiosourceprereadcache_internaldata* idata = a->internaldata;
    memset(idata, 0, sizeof(*idata));
    idata->source = source;

    a->read = &audiosourceprereadcache_Read;
    a->close = &audiosourceprereadcache_Close;
    a->rewind = &audiosourceprereadcache_Rewind;

    return a;
}
