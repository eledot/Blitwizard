
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

#include "audiosource.h"
#include "audiosourcefile.h"

#if defined(ANDROID) || defined(__ANDROID__)
#include "SDL.h"
#endif

struct audiosourcefile_internaldata {
#if defined(ANDROID) || defined(__ANDROID__)
	SDL_RWops* file;
#else
	FILE* file;
#endif
	int eof;
	char* path;
};

static void audiosourcefile_Rewind(struct audiosource* source) {
	struct audiosourcefile_internaldata* idata = source->internaldata;
	idata->eof = 0;
	if (idata->file) {
#if defined(ANDROID) || defined(__ANDROID__)
		SDL_FreeRW(idata->file);
#else
		fclose(idata->file);
#endif
		idata->file = 0;
	}
}

static int audiosourcefile_Read(struct audiosource* source, char* buffer, unsigned int bytes) {
	struct audiosourcefile_internaldata* idata = source->internaldata;
	
	if (idata->file == NULL) {
		if (idata->eof) {
			return -1;
		}
#if defined(ANDROID) || defined(__ANDROID__)
		idata->file = SDL_RWFromFile(idata->path, "r");
#else
		idata->file = fopen(idata->path,"rb");
#endif
		if (!idata->file) {
			idata->file = NULL;
			return -1;
		}
	}

#if defined(ANDROID) || defined(__ANDROID__)
	int bytesread = idata->file->read(idata->file, buffer, 1, bytes);
#else
	int bytesread = fread(buffer, 1, bytes, idata->file);
#endif
	if (bytesread > 0) {
		return bytesread;
	}else{
#if defined(ANDROID) || defined(__ANDROID__)
		SDL_FreeRW(idata->file);
#else
		fclose(idata->file);
#endif
		idata->file = NULL;
		idata->eof = 1;
		if (bytesread < 0) {
			return -1;
		}
		return 0;
	}
}

static void audiosourcefile_Close(struct audiosource* source) {
	struct audiosourcefile_internaldata* idata = source->internaldata;	
	//close file we might have opened
#if defined(ANDROID) || defined(__ANDROID__)
	if (idata->file) {
		SDL_FreeRW(idata->file);
	}
#else
	FILE* r = idata->file;
	if (r) {
		free(r);
	}
#endif
	//free all structs & strings
	if (idata->path) {
		free(idata->path);
	}
	if (source->internaldata) {
		free(source->internaldata);
	}
	free(source);
}

struct audiosource* audiosourcefile_Create(const char* path) {
	struct audiosource* a = malloc(sizeof(*a));
	if (!a) {
		return NULL;
	}
	
	memset(a,0,sizeof(*a));
	a->internaldata = malloc(sizeof(struct audiosourcefile_internaldata));
	if (!a->internaldata) {
		free(a);
		return NULL;
	}
	
	struct audiosourcefile_internaldata* idata = a->internaldata;
	memset(idata, 0, sizeof(*idata));
	idata->path = strdup(path);
	if (!idata->path) {
		free(a->internaldata);
		free(a);
		return NULL;
	}	

	a->read = &audiosourcefile_Read;
	a->close = &audiosourcefile_Close;
	a->rewind = &audiosourcefile_Rewind;
	
	return a;
}

