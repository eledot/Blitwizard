
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "os.h"
#include "audiosource.h"
#include "audiosourceresample.h"

#ifndef USE_SPEEX_RESAMPLING

void audiosourceresample_Close(struct audiosource* source) {
    struct audiosource* internalsource = source->internaldata;
    if (internalsource) {
        internalsource->close(internalsource);
    }
    free(source);
}

void audiosourceresample_Rewind(struct audiosource* source) {
    struct audiosource* internalsource = source->internaldata;
    internalsource->rewind(internalsource);
}

int audiosourceresample_Read(struct audiosource* source, char* buffer, unsigned int bytes) {
    struct audiosource* internalsource = source->internaldata;
    return internalsource->read(internalsource, buffer, bytes);
}

unsigned int audiosourceresample_Position(struct audiosource* source) {
    struct audiosource* internalsource = source->internaldata;
    return internalsource->position(internalsource);
}

unsigned int audiosourceresample_Length(struct audiosource* source) {
    struct audiosource* internalsource = source->internaldata;
    return internalsource->length(internalsource);
}

int audiosourceresample_Seek(struct audiosource* source, unsigned int pos) {
    struct audiosource* internalsource = source->internaldata;
    return internalsource->seek(internalsource, pos);
}

struct audiosource* audiosourceresample_Create(struct audiosource* source, unsigned int targetrate) {
    if (!source || source->samplerate <= 0) {
        return NULL;
    }
    struct audiosource* as = malloc(sizeof(*as));
    if (!as) {
        return NULL;
    }
    memset(as,0,sizeof(*as));
    as->internaldata = source;
    as->read = &audiosourceresample_Read;
    as->close = &audiosourceresample_Close;
    as->rewind = &audiosourceresample_Rewind;
    as->position = &audiosourceresample_Position;
    as->length = &audiosourceresample_Length;
    as->seek = &audiosourceresample_Seek;
    as->samplerate = targetrate;
    as->channels = source->channels;
    as->format = source->format;
    return as;
}

#else

#include <speex/speex_resampler.h>

struct audiosourceresample_internaldata {
    struct audiosource* source;
    unsigned int targetrate;
    int sourceeof;
    int eof;
    int returnerroroneof;

    SpeexResamplerState* st;

    char unprocessedbuf[512];
    unsigned int unprocessedbytes;

    char processedbuf[2048];
    unsigned int processedbytes;
};

static void audiosourceresample_Close(struct audiosource* source) {
    struct audiosourceresample_internaldata* idata = source->internaldata;
    if (idata) {
        // close the processed source
        if (idata->source) {
            idata->source->close(idata->source);
        }

        // close resampler
        if (idata->st) {
            speex_resampler_destroy(idata->st);
        }

        // free all structs
        free(idata);
    }
    free(source);
}

static void audiosourceresample_Rewind(struct audiosource* source) {
    struct audiosourceresample_internaldata* idata = source->internaldata;
    if (!idata->eof || !idata->returnerroroneof) {
        idata->source->rewind(idata->source);
        idata->sourceeof = 0;
        idata->eof = 0;
        idata->returnerroroneof = 0;
        idata->processedbytes = 0;
        if (idata->st) {
            speex_resampler_destroy(idata->st);
            idata->st = NULL;
        }
    }
}


static int audiosourceresample_Read(struct audiosource* source, char* buffer, unsigned int bytes) {
    struct audiosourceresample_internaldata* idata = source->internaldata;
    if (idata->eof) {
        return -1;
    }

    if (!idata->st) {
        int error;
#ifdef ANDROID
        idata->st = speex_resampler_init(idata->source->channels, idata->source->samplerate, source->samplerate, 1, &error);
#else
        idata->st = speex_resampler_init(idata->source->channels, idata->source->samplerate, source->samplerate, 3, &error);
#endif
        if (!idata->st) {
            idata->eof = 1;
            idata->returnerroroneof = 1;
            return -1;
        }
    }

    unsigned int writtenbytes = 0;

    // If we have unreturned processed bytes left, return them
    unsigned int returnprocessed = idata->processedbytes;
    if (returnprocessed > bytes) {
        returnprocessed = bytes;
    }
    if (returnprocessed > 0) {
        memcpy(buffer, idata->processedbuf, returnprocessed);
        if (returnprocessed < idata->processedbytes) {
            // move up remaining bytes in the buffer
            memmove(idata->processedbuf, idata->processedbuf + returnprocessed, sizeof(idata->processedbuf) - returnprocessed);
        }
        idata->processedbytes -= returnprocessed;
        buffer += returnprocessed;
        bytes -= returnprocessed;
        writtenbytes += returnprocessed;
    }

    // Fetch new resampled content:
    while (bytes > 0) {
        unsigned int wantbytes = idata->source->samplerate / 1000;
        unsigned int outbytes = (source->samplerate / 1000) * 2; // doubled to avoid possible speex float bug on output limitation

        // fetch new source data if required
        if (idata->unprocessedbytes < wantbytes) {
            int result = idata->source->read(idata->source, idata->unprocessedbuf + idata->unprocessedbytes, wantbytes - idata->unprocessedbytes);
            if (result < 0) {
                idata->returnerroroneof = 1;
                idata->eof = 1;
                return -1;
            }else{
                if (result == 0) {
                    idata->sourceeof = 1;
                    break;
                }
                idata->unprocessedbytes += result;
                if (idata->unprocessedbytes < wantbytes) {
                    // too short, read again:
                    continue;
                }
            }
        }

        // resample the data we have now:
        unsigned int in = idata->unprocessedbytes;
        unsigned int out = outbytes - idata->processedbytes;

        // We want to have this in nice full channel samples:
        while (in > 0 && in % (sizeof(float) * source->channels) != 0) {
            in--;
        }
        while (out > 0 && out % (sizeof(float) * source->channels) != 0) {
            out--;
        }
        // printf("in, out: %u, %u\n",in,out);
        unsigned int insamples = in/(sizeof(float)*source->channels);
        unsigned int outsamples = out/(sizeof(float)*source->channels);
        // printf("in samples, out samples: %u, %u\n", insamples,outsamples);

        memset(idata->processedbuf + idata->processedbytes, 0, out);
        int error = speex_resampler_process_interleaved_float(idata->st, (const float*)idata->unprocessedbuf, &insamples, (float*)idata->processedbuf + idata->processedbytes, &outsamples);
        if (error == 0) {
            out = outsamples * sizeof(float) * source->channels;
            in = insamples * sizeof(float) * source->channels;
            // printf("result: in, out: %u(%u), %u(%u)\n", in, insamples, out, outsamples);
            // resampling was successful:
            idata->processedbytes += out;
            idata->unprocessedbytes -= in;
            if (idata->unprocessedbytes > 0) {
                // move remaining unprocessed bytes up to beginning of buffer
                memmove(idata->unprocessedbuf, idata->unprocessedbuf + in, sizeof(idata->unprocessedbuf) - in);
            }

            // return bytes:
            unsigned int returnamount = bytes;
            if (returnamount > idata->processedbytes) {
                returnamount = idata->processedbytes;
            }
            if (returnamount > 0) {
                memcpy(buffer, idata->processedbuf, returnamount);
                if (returnamount < idata->processedbytes) {
                    // move remaining processed bytes up to beginning
                    memmove(idata->processedbuf, idata->processedbuf + returnamount, sizeof(idata->processedbuf) - returnamount);
                }
                bytes -= returnamount;
                writtenbytes += returnamount;
                idata->processedbytes -= returnamount;
                buffer += returnamount;
            }
        }else{
            idata->returnerroroneof = 1;
            idata->eof = 1;
            return -1;
        }
    }

    if (writtenbytes > 0) {
        return writtenbytes;
    }else{
        idata->eof = 1;
        if (idata->returnerroroneof) {
            return -1;
        }
        return 0;
    }
}

static size_t audiosourceresample_Length(struct audiosource* source) {
    struct audiosourceresample_internaldata* idata = source->internaldata;

    if (idata->eof && idata->returnerroroneof) {
        return 0;
    }

    return (idata->source->length(idata->source) *
    source->samplerate) / idata->source->samplerate;
}

static size_t audiosourceresample_Position(struct audiosource* source) {
    struct audiosourceresample_internaldata* idata = source->internaldata;

    if (idata->eof && idata->returnerroroneof) {
        return 0;
    }

    return (idata->source->position(idata->source) *
    source->samplerate) / idata->source->samplerate;
}

static int audiosourceresample_Seek(struct audiosource* source, size_t pos) {
    struct audiosourceresample_internaldata* idata = source->internaldata;

    if (!source->seekable || (idata->eof && idata->returnerroroneof)) {
        return 0;
    }

    // check position against valid boundaries:
    size_t tpos = (pos * source->samplerate) / idata->source->samplerate;
    if (tpos > idata->source->length(idata->source)) {
        tpos = idata->source->length(idata->source);
    }

    // try seeking:
    if (idata->source->seek(idata->source, pos)) {
        return 1;
    }
    return 0;
}

struct audiosource* audiosourceresample_Create(struct audiosource* source, unsigned int targetrate) {
    // check for a correct source and usable sample rates
    if (!source || source->format != AUDIOSOURCEFORMAT_F32LE) {
        if (source) {
            source->close(source);
        }
        return NULL;
    }
    if ((source->samplerate < 1000 || source->samplerate > 100000) &&
        (targetrate < 1000 || targetrate > 100000)) {
        // possibly bogus values
        source->close(source);
        return NULL;
    }
    if (source->samplerate == targetrate) {
        return source;
    }

    // amount of channels must be known
    if (source->channels <= 0) {
        source->close(source);
        return NULL;
    }

    // allocate data struct
    struct audiosource* a = malloc(sizeof(*a));
    if (!a) {
        source->close(source);
        return NULL;
    }

    // allocate data struct for internal (hidden) data
    memset(a,0,sizeof(*a));
    a->internaldata = malloc(sizeof(struct audiosourceresample_internaldata));
    if (!a->internaldata) {
        free(a);
        source->close(source);
        return NULL;
    }

    // remember some things
    struct audiosourceresample_internaldata* idata = a->internaldata;
    memset(idata, 0, sizeof(*idata));
    idata->source = source;
    idata->targetrate = targetrate;
    a->samplerate = targetrate;
    a->channels = source->channels;
    a->format = source->format;

    // set function pointers
    a->read = &audiosourceresample_Read;
    a->close = &audiosourceresample_Close;
    a->rewind = &audiosourceresample_Rewind;
    a->position = &audiosourceresample_Position;
    a->length = &audiosourceresample_Length;
    a->seek = &audiosourceresample_Seek;

    // if our source is seekable, we are so too:
    a->seekable = source->seekable;

    // complete!
    return a;
}

#endif // ifdef USE_SPEEX_RESAMPLING

