
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

#include "os.h"
#include "logging.h"
#include "audiosource.h"
#include "audiosourceffmpeg.h"
#include "library.h"
#include "filelist.h"
#include "file.h"

#ifndef USE_FFMPEG

// No FFmpeg support!

struct audiosource* audiosourceffmpeg_Create(struct audiosource* source) {
	return NULL;
}

void audiosourceffmpeg_DisableFFmpeg() {
	//does nothing in this case, obviously
	return;
}

int audiosourceffmpeg_LoadFFmpeg() {
	return 0; //return failure
}

#else

#define AVIOBUFSIZE (AVCODEC_MAX_AUDIO_FRAME_SIZE*2)

#define DECODEBUFSIZE (AVCODEC_MAX_AUDIO_FRAME_SIZE*8)

struct audiosourceffmpeg_internaldata {
	struct audiosource* source;
	int sourceeof;
	int eof;
	int returnerroroneof;

	unsigned char* aviobuf;
	unsigned char* buf;
	unsigned int bufbytes;
	AVCodecContext* codeccontext;
	AVFormatContext* formatcontext;
	AVIOContext* iocontext;
	AVCodec* audiocodec;
	AVStream* audiostream;
};

static int ffmpegopened = 0;
static void* avformatptr;
static void* avcodecptr;
static void* avutilptr;

static void (*ffmpeg_av_register_all)(void);
static AVFormatContext* (*ffmpeg_avformat_alloc_context)(void);
static AVCodecContext* (*ffmpeg_avcodec_alloc_context3)(AVCodec*);
static void (*ffmpeg_av_free)(void*);
static void* (*ffmpeg_av_malloc)(size_t);
static AVIOContext* (*ffmpeg_avio_alloc_context)(
	unsigned char* buffer, int buffer_size, int write_flag,
	void* opaque,
	int(*read_packet)(void *opaque, uint8_t *buf, int buf_size),
	int(*write_packet)(void *opaque, uint8_t *buf, int buf_size),
	int64_t(*seek)(void *opaque, int64_t offset, int whence)
);
static int (*ffmpeg_av_find_stream_info)(AVFormatContext*,AVDictionary**);
static int (*ffmpeg_av_find_best_stream)(AVFormatContext*, enum AVMediaType, int, int, AVCodec**, int);
static int (*ffmpeg_avcodec_open2)(AVCodecContext*, AVCodec*, AVDictionary**);
static int (*ffmpeg_avformat_open_input)(AVFormatContext**, const char*, AVInputFormat*, AVDictionary**);
static int (*ffmpeg_avcodec_decode_audio3)(AVCodecContext* avctx, int16_t* samples, int* frame_size_ptr, AVPacket* avpkt);
static int (*ffmpeg_av_read_frame)(AVFormatContext* s, AVPacket* pkt);
static void (*ffmpeg_av_free_packet)(AVPacket* pkt);
static void (*ffmpeg_av_init_packet)(AVPacket* pkt);
static int (*ffmpeg_av_strerror)(int errnum, char* errbuf, size_t errbuf_size);
static AVCodec* (*ffmpeg_avcodec_find_decoder)(enum CodecID id);

static int loadorfailstate = 0;
static void loadorfail(void** ptr, void* lib, const char* name) {
	if (loadorfailstate) {return;}
	*ptr = library_GetSymbol(lib, name);
	
	if (!*ptr) {
		printwarning("Warning: [FFmpeg] Failed to load symbol: %s",name);
		loadorfailstate = 1;
		return;
	}
}

static int audiosourceffmpeg_LoadFFmpegFunctions() {
	loadorfail((void**)(&ffmpeg_av_register_all), avformatptr, "av_register_all");
	loadorfail((void**)(&ffmpeg_avformat_alloc_context), avformatptr, "avformat_alloc_context");
	loadorfail((void**)(&ffmpeg_avcodec_alloc_context3), avcodecptr, "avcodec_alloc_context3");
	loadorfail((void**)(&ffmpeg_av_free), avcodecptr, "av_free");
	loadorfail((void**)(&ffmpeg_avio_alloc_context), avformatptr, "avio_alloc_context");
	loadorfail((void**)(&ffmpeg_av_malloc), avformatptr, "av_malloc");
	loadorfail((void**)(&ffmpeg_av_find_stream_info), avformatptr, "av_find_stream_info");
	loadorfail((void**)(&ffmpeg_av_find_best_stream), avformatptr, "av_find_best_stream");
	loadorfail((void**)(&ffmpeg_avcodec_open2), avcodecptr, "avcodec_open2");
	loadorfail((void**)(&ffmpeg_avformat_open_input), avformatptr, "avformat_open_input");
	loadorfail((void**)(&ffmpeg_avcodec_decode_audio3), avcodecptr, "avcodec_decode_audio3");
	loadorfail((void**)(&ffmpeg_av_read_frame), avformatptr, "av_read_frame");
	loadorfail((void**)(&ffmpeg_av_free_packet), avcodecptr, "av_free_packet");
	loadorfail((void**)(&ffmpeg_av_init_packet), avcodecptr, "av_init_packet");
	loadorfail((void**)(&ffmpeg_av_strerror), avutilptr, "av_strerror");
	loadorfail((void**)(&ffmpeg_avcodec_find_decoder), avcodecptr, "avcodec_find_decoder");

	if (loadorfailstate) {
		return 0;
	}
	return 1;
}

static int ffmpegreader(void* data, uint8_t* buf, int buf_size) {
	struct audiosource* source = data;
	struct audiosourceffmpeg_internaldata* idata = (struct audiosourceffmpeg_internaldata*)source->internaldata;
	if (idata->sourceeof) {return 0;}
	
	if (idata->source) {
		int i = idata->source->read(idata->source, (void*)buf, (unsigned int)buf_size); 
		if (i < 0) {
			idata->returnerroroneof = 1;
		}
		if (i == 0) {
			idata->sourceeof = 1;
		}
		return i;
	}
	return -1;
}

static int audiosourceffmpeg_InitFFmpeg() {
	//Register all available codecs
	ffmpeg_av_register_all();

	return 1;
}

void audiosourceffmpeg_DisableFFmpeg() {
	if (ffmpegopened != 0) {return;}
	ffmpegopened = -1;
	printinfo("[FFmpeg] Use of FFmpeg has been disabled.");
}

int audiosourceffmpeg_LoadFFmpeg() {
	if (ffmpegopened != 0) {
		if (ffmpegopened == 1) {
			return 1;
		}
		return 0;
	}

	//Load libraries
#ifndef WIN
	avutilptr = library_LoadSearch("libavutil");
    avcodecptr = library_LoadSearch("libavcodec");
    avformatptr = library_LoadSearch("libavformat");
#else
	avutilptr = library_LoadSearch("avutil");
	avcodecptr = library_LoadSearch("avcodec");
	avformatptr = library_Loadsearch("avformat");
#endif

	//Check library load state
	if (!avcodecptr || !avformatptr || !avutilptr) {
		if (avcodecptr) {
			library_Close(avcodecptr);
		}
		if (avformatptr) {
			library_Close(avformatptr);
		}
		if (avutilptr) {
			library_Close(avutilptr);
		}
	
		printinfo("[FFmpeg] Library not found or cannot be loaded, FFmpeg support will be unavailable");
		ffmpegopened = -1;
		return 0;
	}
	
	//Load functions
	if (!audiosourceffmpeg_LoadFFmpegFunctions()) {
		library_Close(avcodecptr);
		library_Close(avformatptr);
		printinfo("[FFmpeg] Library misses one or more expected symbols. FFmpeg support will be unavailable");
		ffmpegopened = -1;
		return 0;
	}

	//Initialise FFmpeg
	if (!audiosourceffmpeg_InitFFmpeg()) {
		library_Close(avcodecptr);
		library_Close(avformatptr);
		printinfo("[FFmpeg] Library initialisation failed. FFmpeg support will be unavailable");
		ffmpegopened = -1;
		return 0;
	}

	printinfo("[FFmpeg] Library successfully loaded.");
	ffmpegopened = 1;	
	return 1;
}

static void audiosourceffmpeg_Rewind(struct audiosource* source) {
	struct audiosourceffmpeg_internaldata* idata = source->internaldata;
	if (idata->returnerroroneof) {return;}	
	idata->eof = 0;
	if (idata->source) {
		idata->source->rewind(idata->source);
	}
	idata->sourceeof = 0;
}

static void audiosourceffmpeg_FreeFFmpegData(struct audiosource* source) {
	if (!audiosourceffmpeg_LoadFFmpeg()) {
		return;
	}
	struct audiosourceffmpeg_internaldata* idata = source->internaldata;
	/*if (idata->codeccontext) { //part of the stream
		ffmpeg_av_free(idata->codeccontext);
        idata->codeccontext = NULL;
	}*/
	if (idata->formatcontext) {
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
		idata->bufbytes = 0;
	}
	/*if (idata->aviobuf) { //it appears this is handled by the iocontext
		ffmpeg_av_free(idata->aviobuf);
		idata->aviobuf = NULL;
	}*/
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
		//Get format context
		idata->formatcontext = ffmpeg_avformat_alloc_context();
		if (!idata->formatcontext) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}

		//av_dict_set(&codec_opts, "request_channels", "2", 0);
		//opts = setup_find_stream_info_opts(ic, codec_opts);

		//Get IO context
		idata->aviobuf = ffmpeg_av_malloc(AVIOBUFSIZE);
		if (!idata->aviobuf) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}
		idata->iocontext = ffmpeg_avio_alloc_context(idata->aviobuf, AVIOBUFSIZE, 0, source, ffmpegreader, NULL, NULL); 
		if (!idata->iocontext) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}
		idata->formatcontext->pb = idata->iocontext;
		idata->formatcontext->iformat = NULL;
		
		//Read format
		if (ffmpeg_avformat_open_input(&idata->formatcontext, "", NULL, NULL) != 0) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}
		
		//Find stream
		int stream = ffmpeg_av_find_best_stream(idata->formatcontext, AVMEDIA_TYPE_AUDIO, -1, -1, &idata->audiocodec, 0);
		if (stream < 0) {
			//We need more detailed stream info to find the stream
			if (ffmpeg_av_find_stream_info(idata->formatcontext, NULL) < 0) {
				audiosourceffmpeg_FatalError(source);
				return -1;
			}
			stream = ffmpeg_av_find_best_stream(idata->formatcontext, AVMEDIA_TYPE_AUDIO, -1, -1, &idata->audiocodec, 0);
			if (stream < 0) {
				audiosourceffmpeg_FatalError(source);
				return -1;
			}
		}

		//Get codec context
		idata->codeccontext = idata->formatcontext->streams[stream]->codec;

		//Find best codec, we don't trust av_find_best_stream on this for now
		idata->audiocodec = ffmpeg_avcodec_find_decoder(idata->codeccontext->codec_id);
		if (!idata->audiocodec) {
			//Format is recognised, but codec unsupported
  			audiosourceffmpeg_FatalError(source);
  			return -1;
		}
	
		//If this isn't an audio stream, we don't want it:
		if (idata->codeccontext->codec_type != AVMEDIA_TYPE_AUDIO) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}

		//Initialise codec. XXX avcodec_open2 is not thread-safe!
		if (ffmpeg_avcodec_open2(idata->codeccontext, idata->audiocodec, NULL) < 0) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}	

		//Remember the stream we want to use
		idata->audiostream = idata->formatcontext->streams[stream];

		//Allocate actual decoding buf
		idata->buf = malloc(DECODEBUFSIZE);
		if (!idata->buf) {
			audiosourceffmpeg_FatalError(source);
			return -1;
		}
		
		//Remember format data
		source->channels = idata->codeccontext->channels;
		source->samplerate = idata->codeccontext->sample_rate;
		printf("4: %u, %u\n", source->channels, source->samplerate);
		
		if (source->channels <= 0 || source->samplerate <= 0) {
			//not a known format apparently
			audiosourceffmpeg_FatalError(source);
			return -1;
		}
	}

	int writtenbytes = 0;
	printf("ffmpeg: reading %d bytes\n",bytes);
	while (bytes > 0) {
		//decode with FFmpeg
		AVPacket packet;
		ffmpeg_av_init_packet(&packet);
		while (idata->bufbytes + AVCODEC_MAX_AUDIO_FRAME_SIZE * 2 < DECODEBUFSIZE && idata->bufbytes < bytes && ffmpeg_av_read_frame(idata->formatcontext, &packet) >= 0) {
            int data_size = AVCODEC_MAX_AUDIO_FRAME_SIZE * 2;
            int size = packet.size;
            while (size > 0) {
                int len = ffmpeg_avcodec_decode_audio3(idata->codeccontext, (int16_t *)((char*)idata->buf + idata->bufbytes), &data_size, &packet);
				idata->bufbytes += len;
				size -= len;
        	}
			printf("decoded - bufbytes is now: %u\n", idata->bufbytes);
     	}
		ffmpeg_av_free_packet(&packet);

		printf("ffmpeg: we have %u bytes\n", idata->bufbytes);

		//check how much we can return
		unsigned int returnbytes = idata->bufbytes;
		if (returnbytes > bytes) {
			returnbytes = bytes;
		}
		
		//return decoded bytes
		if (returnbytes > 0) {
			//write bytes to provided external buffer
			memcpy(buffer, idata->buf, returnbytes);
			buffer += returnbytes;

			//remove written bytes from internal ->buf
			writtenbytes += returnbytes;
			memmove(idata->buf, idata->buf + returnbytes, DECODEBUFSIZE - returnbytes);
			idata->bufbytes -= returnbytes;
			bytes -= returnbytes;
		}else{
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
	}
	printf("ffmpeg: %d bytes read\n",writtenbytes);
	return writtenbytes;
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

#ifdef NOTHREADEDSDLRW
static void audiosourceffmpeg_CloseMainthread(struct audiosource* source) {
	struct audiosourceffmpeg_internaldata* idata = source->internaldata;
    //close audio source we might have opened
    if (idata && idata->source) {
        idata->source->close(idata->source);
		idata->source = NULL;
    }
	audiosourceffmpeg_Close(source);
}
#endif

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
#ifdef NOTHREADEDSDLRW
	a->closemainthread = &audiosourceffmpeg_CloseMainthread;
#endif	

	//ensure proper initialisation of sample rate + channels variables
    audiosourceffmpeg_Read(a, NULL, 0);
    if (idata->eof && idata->returnerroroneof) {
        //There was an error reading this ogg file - probably not ogg (or broken ogg)
#ifdef NOTHREADEDSDLRW
		audiosourceffmpeg_CloseMainthread(a);
#else
        audiosourceffmpeg_Close(a);
#endif
        return NULL;
    }
	
	return a;
}

#endif

