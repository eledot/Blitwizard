
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
	int sourceeof;
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
	unsigned int processedsamplesbytes;
};

static void audiosourcefadepanvol_Rewind(struct audiosource* source) {
	struct audiosourcefadepanvol_internaldata* idata = source->internaldata;
	if (!idata->eof || !idata->returnerroroneof) {
		idata->source->rewind(idata->source);
		idata->sourceeof = 0;
		idata->eof = 0;
		idata->returnerroroneof = 0;
		idata->processedsamplesbytes = 0;
	}
}

static int audiosourcefadepanvol_Read(struct audiosource* source, char* buffer, unsigned int bytes) {
	struct audiosourcefadepanvol_internaldata* idata = source->internaldata;
	if (idata->eof) {
		return -1;
	}
	
	unsigned int byteswritten = 0;
	while (bytes > 0) {
		//see how many samples we want to have minimum
		int stereosamples = bytes / (sizeof(float) * 2);
		if (stereosamples * sizeof(float) * 2 < bytes) {
			stereosamples++;
		}
			
		//get new unprocessed samples
		int unprocessedstart = idata->processedsamplesbytes;
		if (!idata->sourceeof) {
			while (idata->processedsamplesbytes + sizeof(float) * 2 <= sizeof(idata->processedsamplesbuf) && stereosamples > 0) {
				int i = idata->source->read(idata->source, idata->processedsamplesbuf + idata->processedsamplesbytes, sizeof(float) * 2);
				if (i < (int)sizeof(float)*2) {
					if (i < 0) {
						//read function returned error
						idata->returnerroroneof = 1;
					}
					idata->sourceeof = 1;
					break;
				}else{
					idata->processedsamplesbytes += sizeof(float)*2;
				}
				stereosamples--;
			}
		}
	
		//process unprocessed samples
		unsigned int i = unprocessedstart;
		float faderange = (-idata->fadesamplestart + idata->fadesampleend);
		float fadeprogress = idata->fadesampleend;
		while ((int)i <= ((int)idata->processedsamplesbytes - ((int)sizeof(float) * 2))) {
			float leftchannel = *((float*)((char*)idata->processedsamplesbuf+i));
			float rightchannel = *((float*)((float*)((char*)idata->processedsamplesbuf+i))+1);
	
			if (idata->fadesamplestart < 0 || idata->fadesampleend > 0) {
				//calculate fade volume
				idata->vol = idata->fadevaluestart + (idata->fadevalueend - idata->fadevaluestart)*(1 - fadeprogress/faderange);
	
				//increase fade progress
				idata->fadesamplestart--;
				idata->fadesampleend--;
				fadeprogress = idata->fadesampleend;
					
				if (idata->fadesampleend < 0) {
					//fade ended
					idata->vol = idata->fadevalueend;
					idata->fadesamplestart = 0;
					idata->fadesampleend = 0;

					if (idata->terminateafterfade) {
						idata->sourceeof = 1;
						i = idata->processedsamplesbytes;
					}
				}
			}

			//apply volume
			leftchannel = (leftchannel+1)*idata->vol - 1;
			rightchannel = (rightchannel+1)*idata->vol - 1;
	
			//calculate panning
			leftchannel *= (idata->pan+1)/2;
			rightchannel *= 1-(idata->pan+1)/2;
	
			//write floats back
			memcpy(idata->processedsamplesbuf+i, &leftchannel, sizeof(float));
			memcpy(idata->processedsamplesbuf+i+sizeof(float), &rightchannel, sizeof(float));
				
			i += sizeof(float)*2;
		}

		//return from our processed samples
		unsigned int returnbytes = bytes;
		if (returnbytes > idata->processedsamplesbytes) {
			returnbytes = idata->processedsamplesbytes;
		}
		if (returnbytes <= 0) {
			if (byteswritten <= 0) {
				idata->eof = 1;
				if (idata->returnerroroneof) {
					return -1;
				}
				return 0;
			}else{
				return byteswritten;
			}
		}else{
			byteswritten += returnbytes;
			memcpy(buffer, idata->processedsamplesbuf, returnbytes);
			buffer += returnbytes;
			bytes -= returnbytes;
		}
		//move away processed & returned samples
		if (returnbytes > 0) {
			memmove(idata->processedsamplesbuf, idata->processedsamplesbuf + returnbytes, sizeof(idata->processedsamplesbuf) - returnbytes);
			idata->processedsamplesbytes -= returnbytes;
		}
	}
	return byteswritten;
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
		printf("fadepanvol: no source\n");
		//no source given
		return NULL;
	}
	if (source->channels != 2) {
		//we only support stereo audio
		printf("fadepanvol: no stereo\n");
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
	if (!a->internaldata) {
		free(a);
		source->close(source);
		return NULL;
	}
	
	//remember various things
	struct audiosourcefadepanvol_internaldata* idata = a->internaldata;
	memset(idata, 0, sizeof(*idata));
	idata->source = source;
	a->samplerate = source->samplerate;
	a->channels = source->channels;
	
	//function pointers
	a->read = &audiosourcefadepanvol_Read;
	a->close = &audiosourcefadepanvol_Close;
	a->rewind = &audiosourcefadepanvol_Rewind;
	
	return a;
}

void audiosourcefadepanvol_SetPanVol(struct audiosource* source, float vol, float pan) {
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

