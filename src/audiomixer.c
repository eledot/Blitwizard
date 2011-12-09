
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
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "audio.h"
#include "audiosource.h"
#include "audiosourcefadepanvol.h"
#include "audiosourceresample.h"
#include "audiosourceogg.h"
#include "audiosourcefile.h"
#include "audiosourceloop.h"

#define MAXCHANNELS 32

int lastusedsoundid = -1;

struct soundchannel {
	struct audiosource* fadepanvolsource;
	struct audiosource* loopsource;
	
	int priority;
	int id;
};
struct soundchannel channels[MAXCHANNELS];

char mixedaudiobuf[256];
int mixedaudiobuflen = 0;

void audiomixer_Init() {
	memset(&channels,0,sizeof(struct soundchannel) * MAXCHANNELS);
}

static void audiomixer_CancelChannel(int slot) {
	if (channels[slot].loopsource) {
		channels[slot].loopsource->close(channels[slot].loopsource);
		channels[slot].loopsource = NULL;
		channels[slot].fadepanvolsource = NULL;
		channels[slot].id = 0;
	}
}

static int audiomixer_GetFreeChannelSlot(int priority) {
	int i = 0;
	//first, attempt to find an empty slot
	while (i < MAXCHANNELS) {
		if (!channels[i].loopsource) {
			return i;
		}
		i++;
	}
	//then override one
	while (i < MAXCHANNELS) {
		if (channels[i].loopsource) {
			if (channels[i].priority <= priority) {
				audiomixer_CancelChannel(priority);
				return i;
			}
		}
		i++;
	}
	return -1;
}

static int audiomixer_GetChannelSlotById(int id) {
	int i = 0;
	while (i < MAXCHANNELS) {
		if (channels[i].id == id) {
			return i;
		}
		i++;
	}
	return -1;
}

static int audiomixer_FreeSoundId() {
	while (1) {
		lastusedsoundid++;
		if (lastusedsoundid >= INT_MAX-1) {
			lastusedsoundid = -1;
			continue;
		}
		if (audiomixer_GetChannelSlotById(lastusedsoundid)) {
			continue;
		}
		break;
	}
	return lastusedsoundid;
}



int audiomixer_PlaySoundFromDisk(const char* path, int priority, float volume, float panning, float fadeinseconds, int loop) {
	audio_LockAudioThread();
	int id = audiomixer_FreeSoundId();
	int slot = audiomixer_GetFreeChannelSlot(priority);
	if (slot < 0) {
		//all slots are full. do nothing and simply return an unused unplaying id
		audio_UnlockAudioThread();
		return id;
	}
	
	channels[slot].fadepanvolsource = audiosourcefadepanvol_Create(audiosourceresample_Create(audiosourceogg_Create(audiosourcefile_Create(path)), 48000));

	if (!channels[slot].fadepanvolsource) {
		return -1;
	}
	
	audiosourcefadepanvol_SetPanVol(channels[slot].fadepanvolsource, panning, volume);
	if (fadeinseconds > 0) {
		audiosourcefadepanvol_StartFade(channels[slot].fadepanvolsource, fadeinseconds, volume, 0);
	}
	
	channels[slot].loopsource = audiosourceloop_Create(channels[slot].fadepanvolsource);
	if (!channels[slot].loopsource) {
		channels[slot].fadepanvolsource->close(channels[slot].fadepanvolsource);
		channels[slot].fadepanvolsource = NULL;
		return -1;
	}	

	audiosourceloop_SetLooping(channels[slot].loopsource, loop);
	
	audio_UnlockAudioThread();
	return id;
}

static void audiomixer_HandleChannelEOF(int channel, int returnvalue) {
	audiomixer_CancelChannel(channel);
}

#define MIXTYPE float
#define MIXTYPE_FINAL float
#define MIXSIZE 512

char mixbuf[MIXSIZE];
char mixbuf2[MIXSIZE];
int filledmixpartial = 0;
int filledmixfull = 0;

static void audiomixer_RequestMix(unsigned int bytes) { //SOUND THREAD
	unsigned int filledbytes = filledmixpartial + filledmixfull * sizeof(MIXTYPE);
	
	unsigned int formatdifferencefactor = sizeof(MIXTYPE)/sizeof(MIXTYPE_FINAL);
	unsigned int actualbytes = bytes * formatdifferencefactor;

	//calculate the desired amount of samples
	int sampleamount = (actualbytes - filledbytes)/sizeof(MIXTYPE);
	while (sampleamount * sizeof(MIXTYPE) < actualbytes - filledbytes) {
		sampleamount += sizeof(MIXTYPE);
	}

	//limit to possible buffer maximum
	while (sampleamount * sizeof(MIXTYPE) + filledbytes > MIXSIZE) {
		sampleamount--;
	}

	//check if we want any samples at all
	if (sampleamount <= 0) {
		return;
	}

	//clear out the samples first
	memset(mixbuf + filledbytes, 0, sampleamount * sizeof(MIXTYPE));
	
	//cycle all channels and mix them into the buffer
	unsigned int i = 0;
	while (i <= MAXCHANNELS) {
		if (channels[i].loopsource && channels[i].fadepanvolsource) {
			//read bytes
			printf("Reading %u samples/%u bytes for mixing\n",sampleamount, sampleamount * sizeof(MIXTYPE));
			int k = channels[i].loopsource->read(channels[i].loopsource, mixbuf2, sampleamount * sizeof(MIXTYPE));
			printf("k: %d\n",k);
			if (k <= 0) {
				audiomixer_HandleChannelEOF(i, k);
				i++;
				continue;
			}
			
			//see how many samples we can actually mix from this
			unsigned int mixsamples = k / sizeof(MIXTYPE);
			while (mixsamples * sizeof(MIXTYPE) > (unsigned int)k) {
				mixsamples--;
			}
			printf("mixsamples: %u\n",mixsamples);

			//mix samples
			MIXTYPE* mixtarget = (MIXTYPE*)((char*)mixbuf + filledbytes);
			MIXTYPE* mixsource = (MIXTYPE*)mixbuf2;
			unsigned int r = 0;
			while (r < mixsamples) {
				double sourcevalue = *mixsource;
				sourcevalue = (sourcevalue + 1)/2;
				double targetvalue = *mixtarget;
				targetvalue = (targetvalue + 1)/2;
				double result = -(targetvalue * sourcevalue) + (targetvalue + sourcevalue);
				result = (result * 2) - 1;
				*mixtarget = (MIXTYPE)result;
				r++;
				mixsource++;mixtarget++;
				filledmixfull++;
			}
		}
		i++;
	}
}


char* streambuf = NULL;
unsigned int streambuflen = 0;
void* audiomixer_GetBuffer(unsigned int len) { //SOUND THREAD
	printf("Mixing %u bytes...\n",len);
	if (streambuflen != len && (streambuflen < len || streambuflen > len * 2)) {
		if (streambuf) {free(streambuf);}
		streambuf = malloc(len);
		streambuflen = len;
	}
	memset(streambuf, 0, len);

	char* p = streambuf;
	while (len > 0) {
		printf("len: %d\n",len);
		audiomixer_RequestMix(len);
		
		//see how much bytes we can get
		printf("partial: %u, full: %u\n",filledmixpartial, filledmixfull);
		unsigned int filledbytes = filledmixpartial + filledmixfull * sizeof(MIXTYPE);
		unsigned int amount = len;
		if (amount > filledbytes) {
			amount = filledbytes;
		}
		printf("Available bytes for mixing: %u\n",amount);
		if (amount <= 0) {break;}

		//copy the amount of bytes we have
		memcpy(p, mixbuf, amount);
		len -= amount;
		p += amount;

		//trim mix buffer:
		if (amount >= filledbytes) {
			//simply mark mix buffer as empty
			filledmixpartial = 0;
			filledmixfull = 0;
		}else{
			//move back contents
			memmove(mixbuf, mixbuf + amount, sizeof(mixbuf) - amount);

			//update fill state
			unsigned int fullparsedsamples = amount / sizeof(MIXTYPE);
			if (fullparsedsamples > amount) {
				fullparsedsamples -= sizeof(MIXTYPE);
			}
			int partialparsedbytes = amount - fullparsedsamples;
			filledmixfull -= fullparsedsamples;
			filledmixpartial -= partialparsedbytes;
			
			//handle partial bytes correctly
			while (filledmixpartial < 0) {
				filledmixpartial += sizeof(MIXTYPE);
				filledmixfull++;
			}
			while (filledmixpartial >= (int)sizeof(MIXTYPE)) {
				filledmixpartial -= sizeof(MIXTYPE);
				filledmixfull--;
			}
		}

		len -= amount;
	}
	
	printf("Returning mix of %u bytes\n",len);
	return streambuf;
}

