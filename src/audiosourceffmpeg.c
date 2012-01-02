
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

#include "audiosource.h"
#include "audiosourceffmpeg.h"

#ifndef USE_FFMPEG

// No FFmpeg support!

struct audiosource* audiosourceffmpeg_Create(struct audiosource* source) {
	return NULL;
}

#else

struct audiosourceffmpeg_internaldata {
	struct audiosource* source;
	int eof;
	int returnerroroneof;
};

static void audiosourceffmpeg_Rewind(struct audiosource* source) {
	struct audiosourceffmpeg_internaldata* idata = source->internaldata;
	if (i->returnerroroneof) {return;}	
	idata->eof = 0;
	if (idata->source) {
		idata->source->rewind(idata->source);
	}
}

static int audiosourceffmpeg_Read(struct audiosource* source, char* buffer, unsigned int bytes) {
	struct audiosourceffmpeg_internaldata* idata = source->internaldata;
	idata->eof = 1;
	return -1;
}

static void audiosourceffmpeg_Close(struct audiosource* source) {
	struct audiosourceffmpeg_internaldata* idata = source->internaldata;	
	//close audio source we might have opened
	if (idata && idata->source) {
		idata->source->close(idata->source);
	}
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

