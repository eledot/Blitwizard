
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


#include <stdio.h>
#include <stdlib.h>

#include "audiosource.h"
#include "audiosourcefile.h"

struct audiosourcefadepanvol_internaldata {
	struct audiosource* source;
	int eof = 0;
	
	int fadesamplestart;
	int fadesampleend;
	float fadevaluestart;
	float fadevalueend;
	
	float pan;
	float vol;
	
	char processedsamplesbuf[512];
	int processedsamplesbytes;
};

static int audiosourcefadepanvol_Read(struct audiosource* source, unsigned int bytes, char* buffer) {
	struct audiosourcefadepanvol_internaldata* idata = source->internaldata;
	if (idata->eof) {
		return -1;
	}
	
	if (idata->source) {
		
	}else{
		idata->eof = 1;
		return 0;
	}
}

static void audiosourcefadepanvol_Close(struct audiosource* source) {
	struct audiosourceogg_internaldata* idata = source->internaldata;
	
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

struct audiosource* audiosourcefadepanvol_Create(struct audiosource* source) {
	struct audiosource* a = malloc(sizeof(*a));
	if (!a) {
		return NULL;
	}
	
	memset(a,0,sizeof(*a));
	a->internaldata = malloc(sizeof(struct audiosourcefadepanvol_internaldata));
	if (a->internaldata) {
		free(a);
		return NULL;
	}
	memset(a->internaldata, 0, sizeof(*(a->internaldata)));
	
	struct audiosourcefadepanvol_internaldata* idata = a->internaldata;
	idata->source = source;
	a->samplerate = source->samplerate;
	
	a->read = &audiosourcefadepanvol_Read;
	a->close = &audiosourcefadepanvol_Close;
	
	return NULL;
}

void audiosourcefadepanvol_SetPanVol(struct audiosource* source, float pan, float vol) {
	struct audiosourcefadepanvol_internaldata* idata = source->internaldata;
	if (pan < -1) {pan = -1;}
	if (pan > 1) {pan = 1;}
	if (vol < 0) {vol = 0;}
	if (vol > 1) {vol = 1;}
	idata->pan = pan;
	idata->vol = vol;
	
	idata->fadesamplestart = 0;
	idata->fadesampleend = 0;
	idata->fadevalueend = vol;
}

void audiosourcefadepanvol_StartFade(struct audiosource* source, float seconds, float targetvol) {
	struct audiosourcefadepanvol_internaldata* idata = source->internaldata;
	if (pan < -1) {pan = -1;}
	if (pan > 1) {pan = 1;}
	if (vol < 0) {vol = 0;}
	if (vol > 1) {vol = 1;}
	
	idata->fadevaluestart = vol;
	idata->fadevalueend = targetvol;
	idata->fadesamplestart = 0;
	idata->fadesampleend = (int)((double)((double)idata->source->samplerate) * ((double)seconds));
}

