
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
#include "audiosourcefadepanvol.h"

struct audiosourcefadepanvol_internaldata {
	struct audiosource* source;
	int eof;
	int returnerroroneof;
	
	int fadesamplestart;
	int fadesampleend;
	float fadevaluestart;
	float fadevalueend;
	int terminateafterfade;
	
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
	
	int writtenbytes = 0;
	while (bytes > 0) {
		//see how many samples we want to have minimum
		int stereosamples = bytes / sizeof(float) * 2;
		if (stereosamples * sizeof(float) * 2 < bytes) {
			stereosamples++;
		}
			
		//get new unprocessed samples
		if (idata->source) {
			int unprocessedstart = idata->processedsamplesbytes;
			while (idata->processedsamplesbytes + sizeof(float) * 2 <= sizeof(idata->processedsamplesbuf) && stereosamples > 0) {
				int i = idata->source->read(idata->source, sizeof(float) * 2, idata->processedsamplesbuf + idata->processedsamplesbytes);
				if (i < sizeof(float)*2) {
					if (i < 0) {
						//read function returned error
						idata->returnerroroneof = 1;
					}
					idata->eof = 1;
					break;
				}else{
					idata->processedsamplesbytes += sizeof(float)*2;
				}
				stereosamples--;
			}
		}
	
		//process unprocessed samples
		int i = unprocessedstart;
		float faderange = (-idata->fadesamplestart + idata->fadesampleend);
		float fadeprogress = idata->fadesampleend;
		while (i <= idata->processedsamplesbytes - sizeof(float) * 2) {
			float leftchannel = *((float*)((void*)idata->processedsamplesbuf+i));
			float rightchannel = *(((float*)((void*)idata->processedsamplesbuf+i))+1);
	
			if (idata->fadesamplestart < 0 || idata->fadesampleend > 0) {
				//calculate fade volume
				idata->vol = idata->fadevaluestart + (idata->fadevalueend - idata->fadevaluestart)*(1 - fadeprogress/faderange);
	
				//increase fade progress
				idata->fadesamplestart--;
				idata->fadesampleend--;
				fadeprogress = idata->fadesampleend;
					
				if (idata->fadesampleend < 0) {
					//fade ended
					idata->fadesamplestart = 0;
					idata->fadesampleend = 0;

					if (idata->terminateafterfade) {
						idata->source->close(idata->source);
						idata->source = NULL;
						idata->processedsamplesbytes = i + sizeof(float) * 2;
					}
				}
			}

			//apply volume
			leftchannel = (leftchannel+1)*idata->vol - 1;
			rightchannel = (leftchannel+1)*idata->vol - 1;
	
			//calculate panning
			leftchannel *= (idata->pan+1)/2;
			rightchannel *= 1-(idata->pan+1)/2;
	
			//write floats back
			memcpy(idata->processedsamplesbuf+i, &leftchannel, sizeof(float));
			memcpy(idata->processedsamplesbuf+i+sizeof(float), &rightchannel, sizeof(float));
				
			i += sizeof(float)*2;
		}

		//return from our processed samples
		int returnbytes = bytes;
		if (returnbytes > idata->processedsamplesbytes) {
			returnbytes = idata->processedsamplesbytes;
		}
		if (returnbytes <= 0) {
			idata->eof = 1;
			if (idata->returnerroroneof) {
				return -1;
			}
			return 0;
		}else{
			writtenbytes += returnbytes;
			memcpy(buffer, idata->processedsamplesbuf, returnbytes);
			buffer += returnbytes;
		}
		//move away processed & returned samples
		memmove(idata->processedsamplesbuf, idata->processedsamplesbuf + writtenbytes, sizeof(idata->processedsamplesbuf) - writtenbytes);
		idata->processedsamplesbytes -= writtenbytes;
	}
	return writtenbytes;
}

static void audiosourcefadepanvol_Close(struct audiosource* source) {
	struct audiosourcefadepanvol_internaldata* idata = source->internaldata;
	
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
	if (!source) {
		//no source given
		return NULL;
	}
	if (source->channels != 2) {
		//we only support stereo audio
		source->close(source);
		return NULL;
	}

	//allocate visible data struct
	struct audiosource* a = malloc(sizeof(*a));
	if (!a) {
		source->close(source);
		return NULL;
	}
	
	//allocate internal data struct
	memset(a,0,sizeof(*a));
	a->internaldata = malloc(sizeof(struct audiosourcefadepanvol_internaldata));
	if (a->internaldata) {
		free(a);
		source->close(source);
		return NULL;
	}
	memset(a->internaldata, 0, sizeof(*(a->internaldata)));
	
	//remember various things
	struct audiosourcefadepanvol_internaldata* idata = a->internaldata;
	idata->source = source;
	a->samplerate = source->samplerate;
	
	//function pointers
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

void audiosourcefadepanvol_StartFade(struct audiosource* source, float seconds, float targetvol, int terminate) {
	struct audiosourcefadepanvol_internaldata* idata = source->internaldata;
	if (seconds <= 0) {
		idata->fadevaluestart = 0;
		idata->fadevalueend = 0;
		idata->fadesamplestart = 0;
		idata->fadesampleend = 0;
		return;
	}
	if (targetvol < 0) {targetvol = 0;}
	if (targetvol > 1) {targetvol = 1;}
	
	idata->terminateafterfade = terminate;
	idata->fadevaluestart = idata->vol;
	idata->fadevalueend = targetvol;
	idata->fadesamplestart = 0;
	idata->fadesampleend = (int)((double)((double)idata->source->samplerate) * ((double)seconds));
}

