
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
    
    // EOF information of this file source:
    int eof;  // we have spilled out an EOF
    int returnerroroneof;  // keep errors in mind
    
    // buffer for decoded bytes:
    char decodedbuf[512];
    unsigned int decodedbytes;

    // FLAC decoder information:
    FLAC__StreamDecoder* decoder;
    int flacopened;
    int flaceof;  // FLAC decoder has signaled end of stream
};

static void audiosourceflac_Rewind(
struct audiosource* source) {
    struct audiosourceflac_internaldata* idata =
    source->internaldata;
    if (!idata->eof || !idata->returnerroroneof) {
        // free libFLAC decoder data:
        if (idata->flacopened) {
            FLAC__stream_decoder_delete(idata->decoder);
            idata->decoder = NULL;
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

static FLAC__StreamDecoderSeekStatus flacseek(
const FLAC__StreamDecoder* decoder, FLAC__uint64 absolute_byte_offset,
void* client_data) {
    return FLAC__STREAM_DECODER_SEEK_STATUS_UNSUPPORTED;
}

static FLAC__bool flaceof(const FLAC__StreamDecoder* ,
void* client_data) {
    return ((struct audiosourceflac_internaldata*)
    ((struct audiosource*)client_data)->filesourceeof;
}

static FLAC__StreamDecoderReadStatus flacreader(
const FLAC__StreamDecoder* decoder, FLAC__byte buffer[],
size_t* bytes, void* client_data) {
    // read data for the FLAC decoder:
    struct audiosourceflac_internaldata* idata =
    ((struct audiosource*)client_data)->internaldata;
    
    if (idata->filesourceeof) {
        return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
    }
    
    int r = idata->source->read(idata->source,
    buffer, r);
    if (r <= 0) {
        if (r == 0) {
            idata->filesourceeof = 1;
            return FLAC__STREAM_DECODER_READ_STATUS_END_OF_STREAM;
        }
        return FLAC__STREAM_DECODER_READ_STATUS_ABORT;
    }
    *bytes = r;
    return FLAC__STREAM_DECODER_READ_STATUS_CONTINUE;
}

FLAC__StreamDecoderWriteStatus flacwrite(
const FLAC__StreamDecoder* decoder,
const FLAC__Frame* frame, const FLAC__int32* const buffer[],
void *client_data) {
    struct audiosourceflac_internaldata* idata =
    ((struct audiosource*)client_data)->internaldata;
    
    
}


void audiosource_Close(struct audiosource* source) {
    if (idata->flacopened) {
        FLAC__stream_decoder_delete(idata->decoder);
        idata->decoder = NULL;
        idata->flacopened = 0;
    }
    
    if (idata->source) {
        idata->source->close(idata->source);
        idata->source = NULL;
    }
}

static int audiosourceflac_InitFLAC(struct audiosource* source) {
    struct audiosourceflac_internaldata* idata =
    source->internaldata;
    
    // open up FLAC decoder:
    idata->decoder = FLAC__stream_decoder_new();
    if (!idata->decoder) {
        return -1;
    }
    
    if (FLAC__stream_decoder_init_stream(idata->decoder,
    flacread,
    NULL,  // seek
    NULL,  // tell
    NULL,  // length
    flaceof,
    flacwrite,
    NULL,  // metadata
    flacerror,
    source
    ) != FLAC__STREAM_DECODER_INIT_STATUS_OK) {
        
    }
}

static int audiosourceflac_Read(
struct audiosource* source, char* buffer,
unsigned int bytes) {
    struct audiosourceflac_internaldata* idata =
    source->internaldata;
    if (idata->eof) {
        return -1;
    }
    
    // open up flac file if we don't have one yet
    if (!idata->flacopened) {
        if (!audiosourceflac_InitFLAC(source)) {
            idata->eof = 1;
            idata->returnerroroneof = 1;
            source->samplerate = -1;
            source->channels = -1;
            return -1;
        }
        
        // now we want to get the sample rate and channels:
        
        vorbis_info* vi = ov_info(&idata->vorbisfile, -1);
        source->samplerate = vi->rate;
        source->channels = vi->channels;
        if (source->channels < 1 || source->channels > 2) {
            // incompatible channel count
            idata->eof = 1;
            idata->returnerroroneof = 1;
            source->samplerate = -1;
            source->channels = -1;
            ov_clear(&idata->vorbisfile);
            return -1;
        }
        idata->flacopened = 1;
    }
}

struct audiosource* audiosourceflac_Create(struct audiosource* source) {
    // we need a working input source obviously:
    if (!source) {
        return NULL;
    }
    
    // main data structure
    struct audiosource* a = malloc(sizeof(*a));
    if (!a) {
        source->close(source);
        return NULL;
    }
    memset(a,0,sizeof(*a));

    // internal data structure
    a->internaldata = malloc(sizeof(struct audiosourceflac_internaldata));
    if (!a->internaldata) {
        free(a);
        source->close(source);
        return NULL;
    }
    struct audiosourceflac_internaldata* idata =
    a->internaldata;
    
    // take an initial peek at the file:
    audiosourceflac_Read(a, NULL, 0);
    if (idata->eof && idata->returnerroroneof) {
        // There was an error reading this ogg file - probably not ogg (or broken ogg)
        audiosourceogg_Close(a);
        return NULL;
    }
    return NULL;
}

#endif // ifdef USE_FLAC_AUDIO

#endif // ifdef USE_AUDIO
