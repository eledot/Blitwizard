
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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include "audiosource.h"
#include "audiosourceformatconvert.h"
#include "mathhelpers.h"

#define CONVERTBUFSIZE 32

struct audiosourceformatconvert_internaldata {
    struct audiosource* source; //internal audio source for format conversion
    int sourceeof;
    int erroroneof; //error when eof is reached
    int eof;
    int targetformat;
    char convertbuf[CONVERTBUFSIZE];
    int convertbufbytes;
};

static void audiosourceformatconvert_Close(struct audiosource* source) {
    struct audiosourceformatconvert_internaldata* idata = (struct audiosourceformatconvert_internaldata*)source->internaldata;
    if (idata->source) {idata->source->close(idata->source);}
    return;
}

static void audiosourceformatconvert_Rewind(struct audiosource* source) {
    struct audiosourceformatconvert_internaldata* idata = (struct audiosourceformatconvert_internaldata*)source->internaldata;

    if (idata->erroroneof) {
        return;
    }
    idata->source->rewind(idata->source);
    idata->sourceeof = 0;
    idata->eof = 0;
    idata->convertbufbytes = 0;
}


static int audiosourceformatconvert_Read(struct audiosource* source, char* buffer, unsigned int bytes) {
    struct audiosourceformatconvert_internaldata* idata = (struct audiosourceformatconvert_internaldata*)source->internaldata;

    //We cannot convert anything more when we EOF'ed previously
    if (idata->eof) {
        idata->erroroneof = 1;
        return -1;
    }

    int writtenbytes = 0;

    //If we have previously converted bytes, return them:
    if (idata->convertbufbytes > 0 && bytes > 0) {
        unsigned int amount = idata->convertbufbytes;
        if (amount > bytes) {amount = bytes;}
        
        //copy bytes:
        memcpy(buffer, idata->convertbuf, amount);
        idata->convertbufbytes -= amount;
        writtenbytes += amount;
        buffer += amount;
        bytes -= amount;
        
        //move remaining bytes up to beginning:
        if (idata->convertbufbytes > 0) {
            memmove(idata->convertbuf, idata->convertbuf + amount, CONVERTBUFSIZE - amount);
        }
    }

    //fetch new bytes and return them
    while (bytes > 0) {
        if (idata->sourceeof) {
            break;
        }
        
        //check how many bytes we want:
        int wantbytes = 0;
        if (idata->source->format == AUDIOSOURCEFORMAT_S16LE) {
            wantbytes = 2;
        }
        if (idata->source->format == AUDIOSOURCEFORMAT_F32LE) {
            wantbytes = 4;
        }

        char bytebuf[8];
        //read the bytes we want:
        int result = idata->source->read(idata->source, bytebuf, wantbytes);
        if (result < wantbytes) {
            if (result < 0) {
                idata->erroroneof = 1;
            }else{
                idata->sourceeof = 1;
                break;
            }
        }

        //convert:
        if (idata->source->format == AUDIOSOURCEFORMAT_S16LE &&
        idata->targetformat == AUDIOSOURCEFORMAT_F32LE) {
            double intmax_big = 32768;
            //convert s16le > f32le
            int16_t old = *(int16_t*)((int16_t*)bytebuf);
            double convert = old;
            convert /= intmax_big;
            float new = convert;

            //copy the result into our buffer
            memcpy(idata->convertbuf, &new, sizeof(new));
            idata->convertbufbytes += sizeof(new);
        }
        if (idata->source->format == AUDIOSOURCEFORMAT_F32LE &&
        idata->targetformat == AUDIOSOURCEFORMAT_S16LE) {
            double intmax_small = 32767;
            //convert f32le > s16le
            float old = *((float*)bytebuf);
            double convert = old;
            convert *= intmax_small;
            int16_t new = (int16_t)fastdoubletoint32(convert);

            //copy the result into our buffer
            memcpy(idata->convertbuf, &new, sizeof(new));
            idata->convertbufbytes += sizeof(new);
        }

        //return converted bytes;
        unsigned int amount = idata->convertbufbytes;
        if (amount > bytes) {
            amount = bytes;
        }
        memcpy(buffer, idata->convertbuf, amount);
        writtenbytes += amount;
        buffer += amount;
        bytes -= amount;
        idata->convertbufbytes -= amount;
        //we served all requested bytes:
        if (bytes == 0) {
            //are some bytes left in buffer?
            if (idata->convertbufbytes > 0) {
                //move them up to the beginning
                memmove(idata->convertbuf, idata->convertbuf + amount, CONVERTBUFSIZE - amount);
            }
            //all requested bytes served:
            break;
        }
    }

    if (writtenbytes <= 0 && idata->sourceeof) {
        idata->eof = 1;
        if (idata->erroroneof) {
            return -1;
        }
        return 0;
    }else{
        return writtenbytes;
    }
}

static int audiosourceformatconvert_Seek(struct audiosource* source, unsigned int pos) {
    //Forward the seke to our audio source if possible:
    struct audiosourceformatconvert_internaldata* idata = (struct audiosourceformatconvert_internaldata*)source->internaldata;

    if (idata->sourceeof) {return 0;}
    if (!idata->source->seek) {return 0;}
    return idata->source->seek(idata->source, pos);
}

static unsigned int audiosourceformatconvert_Position(struct audiosource* source) {
    //Forward the position query to our audio source:
    struct audiosourceformatconvert_internaldata* idata = (struct audiosourceformatconvert_internaldata*)source->internaldata;

    return idata->source->position(idata->source);
}

static unsigned int audiosourceformatconvert_Length(struct audiosource* source) {
    //Forward the length query to our audio source:
    struct audiosourceformatconvert_internaldata* idata = (struct audiosourceformatconvert_internaldata*)source->internaldata;    

    return idata->source->length(idata->source);
}

struct audiosource* audiosourceformatconvert_Create(struct audiosource* source, unsigned int newformat) {
    //Check some obvious cases
    if (newformat == AUDIOSOURCEFORMAT_UNKNOWN) {
        return NULL;
    }
    if (!source || source->format == AUDIOSOURCEFORMAT_UNKNOWN || source->samplerate == 0) {
        return NULL;
    }
    if (source->format == newformat) {
        return source;
    }
   
    //Allocate audio source struct 
    struct audiosource* a = malloc(sizeof(*a));
    if (!a) {
        source->close(source);
        return NULL;
    }
    memset(a, 0, sizeof(*a));

    //Prepare internal data struct
    a->internaldata = malloc(sizeof(struct audiosourceformatconvert_internaldata));
    if (!a->internaldata) {
        free(a);
        source->close(source);
        return NULL;
    }
    struct audiosourceformatconvert_internaldata* idata = a->internaldata;
    memset(idata, 0, sizeof(*idata));    

    //Remember some internal info:
    idata->source = source;
    idata->targetformat = newformat;
    a->format = newformat;
    a->channels = source->channels;
    a->samplerate = source->samplerate;

    //Set callbacks:
    a->read = &audiosourceformatconvert_Read;
    a->length = &audiosourceformatconvert_Length;
    a->position = &audiosourceformatconvert_Position;
    a->close = &audiosourceformatconvert_Close;
    a->seek = &audiosourceformatconvert_Seek;
    a->rewind = &audiosourceformatconvert_Rewind;
    return a;
}

