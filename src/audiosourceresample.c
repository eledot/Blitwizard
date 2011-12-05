
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "audiosource.h"
#include "audiosourceresample.h"

struct audiosourceresample_internaldata {
	struct audiosource* source;
	int targetrate;
	int sourceeof;
	int eof;
	int returnerroroneof;

	char unprocessedbuf[4096];
	int unprocessedbytes;
	int donotusebytes;

	char processedbuf[4096];
	int processedbytes;
};

static void audiosourceresample_Close(struct audiosource* source) {
	struct audiosourceresample_internaldata* idata = source->internaldata;
	
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

static void audiosourcefadepanvol_Rewind(struct audiosource* source) {
	struct audiosourceresample_internaldata* idata = source->internaldata;
	if (!idata->eof || !idata->returnerroroneof) {
		idata->source->rewind(idata->source);
		idata->sourceeof = 0;
		idata->eof = 0;
		idata->returnerroroneof = 0;
		idata->unprocessedbytes = 0;
		idata->processedbytes = 0;
	}
}

static void audiosourceresample_AppendResampled(struct audiosource* source, char* buffer, int size) {
	struct audiosourceresample_internaldata* idata = source->internaldata;
	memcpy(idata->processedbuf + idata->processedbytes, buffer, size);
	idata->processedbytes += size;
}

static int audiosourceresample_GetResampleSpace(struct audiosource* source) {
	struct audiosourceresample_internaldata* idata = source->internaldata;
	return sizeof(idata->processedbuf) - idata->processedbytes;
}

static int audiosourceresample_Read(struct audiosource* source, char* buffer, unsigned int bytes) {
	struct audiosourceresample_internaldata* idata = source->internaldata;
	if (idata->eof) {
		return -1;
	}
	
	int byteswritten = 0;
	while (bytes > 0) {
		if (!idata->sourceeof) {
			//see how many samples we want to fetch
			int wantsamples = bytes / (sizeof(float) * source->channels);
			if (wantsamples < sizeof(float) * source->channels * bytes) {
				wantsamples += sizeof(float) * source->channels;
			}
			if (wantsamples < 12 && (source->samplerate == 44100 ||source->samplerate == 22050) && target->samplerate == 48000) {
				wantsamples = 12;
			}

			//fetch new bytes from the source
			while (wantsamples > 0 && idata->unprocessedbytes < sizeof(idata->unprocessedbuf) - sizeof(float) * source->channels) {
				int i = idata->source->read(idata->source, idata->unprocessedbuf + idata->unprocessedbytes, sizeof(float) * source->channels);
				if (i > 0) {
					idata->unprocessedbytes += i;
				}else{
					if (i < 0) {
						idata->returnerroroneof = 1;
					}
					idata->sourceeof = 1;
					break;
				}
				wantsamples--;
			}
		}
		
		char zerobuf[] = "\0\0\0\0\0\0\0\0"; //contains some zeroes
		
		//resample 22050 -> 44100
		if (source->samplerate == 22050 && target->samplerate == 44100) {
			char* offsetptr = idata->unprocessedbuf;
			int availablebytes = idata->unprocessedbytes;
			int processed = 0;
			while (availablebytes >= sizeof(float) * source->channels && audiosourcesample_GetResampleSpace(source) > sizeof(float) * source->channels * 2) {
				//first, copy the original block of samples unchanged:
				audiosourceresample_AppendResampled(source, offsetptr, sizeof(float) * source->channels);
				//blow up each sample to two samples by adding in a zero block:
				int i = 0;
				while (i < source->channels) {
					audiosourceresample_AppendResampled(source, zerobuf, sizeof(float));
					i++;
				}
				//update status
				availablebytes -= sizeof(float) * source->channels;
				offsetptr += sizeof(float) * source->channels;
				processed += sizeof(float) * source->channels;
			}
			
			//strip the now processed data
			memmove(idata->unprocessedbuf, idata->unprocessedbuf, + processed, sizeof(idata->unprocessedbuf) - processed);
			idata->unprocessedbytes -= processed;
		}
		
		//resample 44100 -> 48000
		if (source->samplerate == 44100 && target->samplerate == 48000) {
			//we use an upsample factor of 11. obviously, that is inaccurate
			char* offsetptr = idata->unprocessedbuf;
			int availablebytes = idata->unprocessedbytes;
			int processed = 0;
			while (audiosourceresample_GetResampleSpace(source) >= 12 * sizeof(float) * source->channels && availablebytes >= 11 * sizeof(float) * source->channels) {
				//first, copy the original block of samples unchanged:
				audiosourceresample_AppendResampled(source, offsetptr, sizeof(float) * source->channels * 11);
				
				//add in an additional zero:
				int i = 0;
				while (i < source->channels) {
					audiosourceresample_AppendResampled(source, zerobuf, sizeof(float));
					i++;
				}
				
				//update status
				availablebytes -= sizeof(float) * source->channels * 11;
				offsetptr += sizeof(float) * source->channels * 11;
				processed += sizeof(float) * source->channels * 11;
			}
			
			//strip the now processed data
			memmove(idata->unprocessedbuf, idata->unprocessedbuf, + processed, sizeof(idata->unprocessedbuf) - processed);
			idata->unprocessedbytes -= processed;
		}
		
		//now we would love to do a lowpass filter. however, this remains to be coded!
		
		
		//serve processed bytes
		int i = bytes;
		if (i > idata->processedbytes) {i = idata->processedbytes;}
		if (i <= 0) {
			if (byteswritten <= 0) {
				if (idata->returnerroroneof) {
					return -1;
				}
				return 0;
			}else{
				return byteswritten;
			}
		}else{
			memcpy(buffer, idata->processedbuf, i);
			memmove(idata->processedbuf, idata->processedbuf + i, sizeof(idata->processedbuf) - i);
			idata->processedbytes -= i;
			byteswritten += i;
		}
	}
	return byteswritten;
}

struct audiosource* audiosourceresample_Create(struct audiosource* source, int targetrate) {
	//check if we got a source and if source samplerate + target rate are supported by our limited implementation
	if (!source) {return NULL;}
	if ((source->samplerate != 44100 || targetsamplerate != 48000) &&
		(source->samplerate != 22050 || targetsamplerate != 44100) &&
		source->samplerate != targetsamplerate) {
		if (source->samplerate == 22050 && targetsamplerate == 48000) {
			//do this as a two-step conversion
			source = audiosourceresample_Create(source, 44100);
			if (!source) {return NULL;}
		}else{
			//sorry, we don't support this.
			source->close(source);
		}
		return NULL;
	}

	//allocate data struct
	struct audiosource* a = malloc(sizeof(*a));
	if (!a) {
		source->close(source);
		return NULL;
	}
	
	//allocate data struct for internal (hidden) data
	memset(a,0,sizeof(*a));
	a->internaldata = malloc(sizeof(struct audiosourceresample_internaldata));
	if (a->internaldata) {
		free(a);
		source->close(source);
		return NULL;
	}
	memset(a->internaldata, 0, sizeof(*(a->internaldata)));
	
	//remember some things
	struct audiosourceresample_internaldata* idata = a->internaldata;
	idata->source = source;
	idata->targetrate = targetrate;
	a->samplerate = source->samplerate;
	
	//set function pointers
	a->read = &audiosourceresample_Read;
	a->close = &audiosourceresample_Close;
	a->rewind = &audiosourcefadepanvol_Rewind;
	
	//complete!
	return a;
}


