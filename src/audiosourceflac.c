
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

//#include "os.h"

#ifdef USE_AUDIO

#ifdef USE_FLAC_AUDIO
#include "FLAC/stream_decoder.h"
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "logging.h"
#include "audiosource.h"
#include "audiosourceflac.h"
#include "library.h"
#include "filelist.h"
#include "file.h"

#ifndef USE_FLAC_AUDIO

//  No FLAC support!

struct audiosource* audiosourceflac_Create(struct audiosource* source) {
    if (source) {
        source->close(source);
    }
    return NULL;
}

#warning "NO FLAC"

#else // ifdef USE_FLAC_AUDIO

// FLAC decoder using libFLAC

struct audiosourceflac_internaldata {
    // file source (or whereever the undecoded audio comes from):
    struct audiosource* source;
    int filesourceeof;  // "file" audio source has given us an EOF
    
    // buffer for raw files requested from the file source:
    char fetchedbuf[4096];
    unsigned int fetchedbytes;
    unsigned int fetchedbufreadoffset;
    
    // EOF information of this file source:
    int eof;  // we have spilled out an EOF
    int returnerroroneof;  // keep errors in mind
    
    // buffer for decoded bytes:
    char decodedbuf[512];
    unsigned int decodedbytes;

    // FLAC decoder information:
    int flacopened;
    int flaceof;  // FLAC decoder has signaled end of stream
};

static void audiosourceogg_Rewind(struct audiosource* source) {
    struct audiosourceogg_internaldata* idata = source->internaldata;
    if (!idata->eof || !idata->returnerroroneof) {
        // free libFLAC decoder data:
        if (idata->flacopened) {
            
            
            
            idata->flacopened = 0;
        }
        
        // rewind file source:
        idata->filesource->rewind(idata->filesource);
        idata->filesourceeof = 0;
        idata->eof = 0;
        idata->returnerroroneof = 0;
        
        // reset buffers:
        idata->fetchedbufreadoffset = 0;
        idata->fetchedbytes = 0;
        idata->decodedbytes = 0;
    }
}

struct audiosource* audiosourceflac_Create(struct audiosource* source) {
    // we need a working input source obviously:
    if (!source) {
        return NULL;
    }
    
    // allocate internal data structure:
    return NULL;
}

#endif // ifdef USE_FLAC_AUDIO

#endif // ifdef USE_AUDIO
