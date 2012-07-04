
/* blitwizard 2d engine - source code file

  Copyright (C) 2011 Jonas Thiem

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
#include <stdlib.h>

#include "os.h"
#include "audiosource.h"
#include "audiosourceloop.h"

struct audiosourceloop_internaldata {
    struct audiosource* source;
    int sourceeof;
    int eof;
    int returnerroroneof;

    int looping;
};

void audiosourceloop_SetLooping(struct audiosource* source, int looping) {
    struct audiosourceloop_internaldata* idata = source->internaldata;
    idata->looping = looping;
}

static void audiosourceloop_Rewind(struct audiosource* source) {
    //this audio source has no rewind functionality
    return;
}

static int audiosourceloop_Read(struct audiosource* source, char* buffer, unsigned int bytes) {
    struct audiosourceloop_internaldata* idata = source->internaldata;
    if (idata->eof) {
        return -1;
    }

    int rewinded = 0; //avoid endless rewind loops for empty sources
    unsigned int byteswritten = 0;
    while (bytes > 0) {
        int i = 0;
        if (!idata->sourceeof) {
            i = idata->source->read(idata->source, buffer, bytes);
            if (i == 0 && idata->looping == 1 && !rewinded) {
                rewinded = 1;
                idata->sourceeof = 0;
                idata->source->rewind(idata->source);
                continue;
            }
        }
        if (i > 0) {
            byteswritten += i;
            buffer += i;
            bytes -= i;
            rewinded = 0;
        }else{
            if (i <= 0) {
                if (i < 0) {
                    idata->returnerroroneof = 1;
                }
                idata->sourceeof = 1;
                if (byteswritten == 0) {
                    idata->eof = 1;
                    if (idata->returnerroroneof) {
                        return -1;
                    }else{
                        return 0;
                    }
                }
                return byteswritten;
            }
        }
    }
    return byteswritten;
}

static void audiosourceloop_Close(struct audiosource* source) {
    struct audiosourceloop_internaldata* idata = source->internaldata;

    //close the processed source
    if (idata->source) {
        idata->source->close(idata->source);
    }

    //free all structs
    if (source->internaldata) {
        free(source->internaldata);
    }
    free(source);
}

#ifdef NOTHREADEDSDLRW
static void audiosourceloop_CloseMainthread(struct audiosource* source) {
    struct audiosourceloop_internaldata* idata = source->internaldata;

    //close the processed source
    if (idata->source) {
        idata->source->closemainthread(idata->source);
        idata->source = NULL;
    }

    audiosourceloop_CloseMainthread(source);
}
#endif

struct audiosource* audiosourceloop_Create(struct audiosource* source) {
    if (!source) {
        //no source given
        return NULL;
    }
    if (source->channels != 2) {
        //we only support stereo audio
#ifdef NOTHREADEDSDLRW
        source->closemainthread(source);
#else
        source->close(source);
#endif
        return NULL;
    }

    //allocate visible data struct
    struct audiosource* a = malloc(sizeof(*a));
    if (!a) {
#ifdef NOTHREADEDSDLRW
        source->closemainthread(source);
#else
        source->close(source);
#endif
        return NULL;
    }

    //allocate internal data struct
    memset(a,0,sizeof(*a));
    a->internaldata = malloc(sizeof(struct audiosourceloop_internaldata));
    if (!a->internaldata) {
        free(a);
#ifdef NOTHREADEDSDLRW
        source->closemainthread(source);
#else
        source->close(source);
#endif
        return NULL;
    }

    //remember various things
    struct audiosourceloop_internaldata* idata = a->internaldata;
    memset(idata, 0, sizeof(*idata));
    idata->source = source;
    a->samplerate = source->samplerate;
    a->channels = source->channels;

    //function pointers
    a->read = &audiosourceloop_Read;
    a->close = &audiosourceloop_Close;
    a->rewind = &audiosourceloop_Rewind;
#ifdef NOTHREADEDSDLRW
    a->closemainthread = &audiosourceloop_CloseMainthread;
#endif

    return a;
}
