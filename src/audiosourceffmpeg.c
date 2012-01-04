
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

#ifdef USE_FFMPEG
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "audiosource.h"
#include "audiosourceffmpeg.h"
#include "library.h"

#ifndef USE_FFMPEG

// No FFmpeg support!

struct audiosource* audiosourceffmpeg_Create(struct audiosource* source) {
	return NULL;
}

#else

#define DECODEBUFSIZE (512+FF_INPUT_BUFFER_PADDING_SIZE)
#define PROBEBUFSIZE 4096

struct audiosourceffmpeg_internaldata {
	struct audiosource* source;
	int eof;
	int returnerroroneof;

	unsigned char* buf;
	AVCodecContext* codeccontext;
	AVFormatContext* formatcontext;
	AVIOContext* iocontext;
	AVProbeData probedata;
};

int ffmpegopened = 0;
void* avformatptr;
void* avcodecptr;
void* avutilptr;

void (*ffmpeg_av_register_all)(void);
AVFormatContext* (*ffmpeg_avformat_alloc_context)(void);
AVCodecContext* (*ffmpeg_avcodec_alloc_context)(void);
void (*ffmpeg_av_free)(void*);
void* (*ffmpeg_av_malloc)(void);
AVIOContext* (*ffmpeg_avio_alloc_context)(void);

int loadorfailstate = 0;
static void loadorfail(void** ptr, void* lib, const char* name) {
	if (loadorfailstate) {return;}
	*ptr = library_GetSymbol(lib, name);
	
	if (!*ptr) {
		loadorfailstate = 1;
		return;
	}
}

static int audiosourceffmpeg_LoadFFmpegFunctions() {
	loadorfail(&ffmpeg_av_register_all, avformatptr, "av_register_all");
	loadorfail(&ffmpeg_avformat_alloc_context, avformatptr, "avformat_alloc_context");
	loadorfail(&ffmpeg_avcodec_alloc_context, avcodecptr, "avcodec_alloc_context");
	loadorfail(&ffmpeg_av_free, avcodecptr, "av_free");
	loadorfail(&ffmpeg_avio_alloc_context, avformatptr, "avio_alloc_context");
	loadorfail(&ffmpeg_av_malloc, avformatptr, "av_malloc");

	if (loadorfailstate) {
		return 0;
	}
	return 1;
}

static int ffmpegreader(void* data, uint8_t* buf, int buf_size) {

}

static int audiosourceffmpeg_InitFFmpeg() {
	//Register all available codecs
	ffmpeg_av_register_all();

	return 1;
}

int audiosourceffmpg_LoadFFmpeg() {
	if (ffmpegopened != 0) {
		if (ffmpegopened == 1) {
			return 1;
		}
		return 0;
	}

	//Load libraries
	avcodecptr = library_Load("libavcodec");
	avformatptr = library_Load("libavformat");
	//avutilptr = library_Load("libavutil");

	//Check library load state
	if (!avcodecptr || !avformatptr) { // || !avutilptr) {
		if (avcodecptr) {
			library_Close(avcodecptr);
		}
		if (avformatptr) {
			library_Close(avformatptr);
		}
		/*if (avutilptr) {
			library_Close(avutilptr);
		}*/
		ffmpegopened = -1;
		return 0;
	}
	
	//Load functions
	if (!audiosourceffmpeg_LoadFFmpegFunctions()) {
		library_Close(avcodecptr);
		library_Close(avformatptr);
		ffmpegopened = -1;
		return 0;
	}

	//Initialise FFmpeg
	if (!audiosourceffmpeg_InitFFmpeg()) {
		library_Close(avcodecptr);
		library_Close(avformatptr);
		ffmpegopened = -1;
		return 0;
	}

	return 1;
}

static void audiosourceffmpeg_Rewind(struct audiosource* source) {
	struct audiosourceffmpeg_internaldata* idata = source->internaldata;
	if (i->returnerroroneof) {return;}	
	idata->eof = 0;
	if (idata->source) {
		idata->source->rewind(idata->source);
	}
}

static void audiosourceffmpeg_FreeFFmpegData(struct audiosource* source) {
	if (!audiosourceffmpeg_LoadFFmpeg()) {
		return;
	}
	struct audiosourceffmpeg_internaldata* idata = source->internaldata;
	if (idata->codeccontext) {
		ffmpeg_av_free(idata->codeccontext);
        idata->codeccontext = NULL;
	}
	if (idata->codeccontext) {
        ffmpeg_av_free(idata->formatcontext);
        idata->formatcontext = NULL;
	}
	if (idata->iocontext) {
		ffmpeg_av_free(idata->iocontext);
		idata->iocontext = NULL;
	}
	if (idata->buf) {
		ffmpeg_av_free(idata->buf);
		idata->buf = NULL;
	}
}

static void audiosourceffmpeg_FatalError(struct audiosource* source) {
	audiosourceffmpeg_FreeFFmpegData(source);
	struct audiosourceffmpeg_internaldata* idata = source->internaldata;
	idata->eof = 1;
	idata->returnerroroneof = 1;
}

static int audiosourceffmpeg_Read(struct audiosource* source, char* buffer, unsigned int bytes) {
	struct audiosourceffmpeg_internaldata* idata = source->internaldata;

	if (idata->eof) {
		return -1;
	}	

	if (!audiosourceffmpeg_LoadFFmpeg()) {
		audiosourceffmpeg_FatalError(source);
		return -1;
	}

	if (!idata->codeccontext) {
		//Get format and codec contexts
		idata->codeccontext = ffmpeg_avcodec_alloc_context();
		if (!idata->codeccontext) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}
		idata->formatcontex = ffmpeg_avformat_alloc_context();
		if (!idata->formatcontext) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}

		//Get IO context
		idata->buf = ffmpeg_av_malloc(DECODEBUFSIZE);
		if (!idata->buf) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}
		idata->iocontext = avio_alloc_context(idata->buf, DECODEBUFSIZE, NULL, source, ffmpegreader, NULL, NULL); 
		if (!idata->iocontext) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}
		idata->formatcontext->br = idata->iocontext;
		idata->formatcontext->iformat = NULL;

		//Read format
		if (avformat_open_input(&idata->formatcontext, "", NULL, NULL) != 0) {
			audiosourceffmpeg_FatalError(source);
		}

		//Probe input format
		
		//Cleanup probing


		//Allocate actual decoding buf
		idata->buf = malloc(DECODEBUFSIZE);
		if (!idata->buf) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}
	}

	return -1;
}

static void audiosourceffmpeg_Close(struct audiosource* source) {
	struct audiosourceffmpeg_internaldata* idata = source->internaldata;	
	//close audio source we might have opened
	if (idata && idata->source) {
		idata->source->close(idata->source);
	}
	//close FFmpeg stuff
	audiosourceffmpeg_FreeFFmpegData(source);
	//free all structs & strings
	if (idata) {
		free(idata);
	}
	free(source);
}

struct audiosource* audiosourceffmpeg_Create(struct audiosource* source) {
	struct audiosource* a = malloc(sizeof(*a));
	if (!a) {
		return NULL;
	}
	if (!source) {
		return NULL;
	}
	
	memset(a,0,sizeof(*a));
	a->internaldata = malloc(sizeof(struct audiosourceffmpeg_internaldata));
	if (!a->internaldata) {
		free(a);
		return NULL;
	}
	
	struct audiosourceffmpeg_internaldata* idata = a->internaldata;
	memset(idata, 0, sizeof(*idata));
	idata->source = source;	

	a->read = &audiosourceffmpeg_Read;
	a->close = &audiosourceffmpeg_Close;
	a->rewind = &audiosourceffmpeg_Rewind;

	//ensure proper initialisation of sample rate + channels variables
    audiosourceffmpeg_Read(a, NULL, 0);
    if (idata->eof && idata->returnerroroneof) {
        //There was an error reading this ogg file - probably not ogg (or broken ogg)
        audiosourceffmpeg_Close(a);
        return NULL;
    }
	
	return a;
}

#endif

