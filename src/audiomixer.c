
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
#include <stdint.h>

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
	struct audiosource* mixsource;
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
	if (channels[slot].mixsource) {
		channels[slot].mixsource->close(channels[slot].mixsource);
		channels[slot].mixsource = NULL;
		channels[slot].loopsource = NULL;
		channels[slot].fadepanvolsource = NULL;
		channels[slot].id = 0;
	}
}

static int audiomixer_GetFreeChannelSlot(int priority) {
	int i = 0;
	//first, attempt to find an empty slot
	while (i < MAXCHANNELS) {
		if (!channels[i].mixsource) {
			return i;
		}
		i++;
	}
	//then override one
	i = 0;
	while (i < MAXCHANNELS) {
		if (channels[i].mixsource) {
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
		if (channels[i].id == id && channels[i].mixsource) {
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
		if (audiomixer_GetChannelSlotById(lastusedsoundid) >= 0) {
			continue;
		}
		break;
	}
	return lastusedsoundid;
}

int audiomixer_IsSoundPlaying(int id) {
	audio_LockAudioThread();
	if (audiomixer_GetChannelSlotById(id) >= 0) {
		audio_UnlockAudioThread();
		return 1;
	}
	audio_UnlockAudioThread();
	return 0;
}

void audiomixer_StopSound(int id) {
	audio_LockAudioThread();
	int slot = audiomixer_GetChannelSlotById(id);
	if (slot >= 0) {
		audiomixer_CancelChannel(slot);
	}
	audio_UnlockAudioThread();
}

void audiomixer_AdjustSound(int id, float volume, float panning) {
	audio_LockAudioThread();
	int slot = audiomixer_GetChannelSlotById(id);
	if (slot >= 0) {
		audiosourcefadepanvol_SetPanVol(channels[slot].fadepanvolsource, volume, panning);
	}
	audio_UnlockAudioThread();
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

	//check audio format first
	struct audiosource* decodesource = audiosourceogg_Create(audiosourcefile_Create(path));
	if (!decodesource) {
		decodesource = audiosourceffmpeg_Create(audiosourcefile_Create(path));
		if (!decodesource) {
			//unsupported audio format
			audio_UnlockAudioThread();
			return -1;
		}
	}
	
	//wrap up the decoded audio into the resampler and fade/pan/vol modifier
	channels[slot].fadepanvolsource = audiosourcefadepanvol_Create(audiosourceresample_Create(decodesource, 48000));

	if (!channels[slot].fadepanvolsource) {
		audio_UnlockAudioThread();
		return -1;
	}
	
	//set the options for the fade/pan/vol modifier
	audiosourcefadepanvol_SetPanVol(channels[slot].fadepanvolsource, volume, panning);
	if (fadeinseconds > 0) {
		audiosourcefadepanvol_StartFade(channels[slot].fadepanvolsource, fadeinseconds, volume, 0);
	}
	
	//wrap the fade/pan/vol modifier into a loop audio source
	channels[slot].loopsource = audiosourceloop_Create(channels[slot].fadepanvolsource);
	if (!channels[slot].loopsource) {
		channels[slot].fadepanvolsource->close(channels[slot].fadepanvolsource);
		channels[slot].fadepanvolsource = NULL;
		audio_UnlockAudioThread();
		return -1;
	}	

	//set the options for the loop audio source
	audiosourceloop_SetLooping(channels[slot].loopsource, loop);
	
	//remember that loop audio source as final processed audio
	channels[slot].mixsource = channels[slot].loopsource;

	audio_UnlockAudioThread();
	return id;
}

static void audiomixer_HandleChannelEOF(int channel, int returnvalue) {
	if (returnvalue) {
		//FIXME: we probably want to emit some sort of warning here
	}
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

	//keep in mind how much of the buffer is properly initialised
	unsigned int bufferinitialisedsamples = 0;
	
	//cycle all channels and mix them into the buffer
	unsigned int i = 0;
	while (i < MAXCHANNELS) {
		if (channels[i].mixsource) {
			//read bytes
			int k = channels[i].mixsource->read(channels[i].mixsource, mixbuf2, sampleamount * sizeof(float));
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

			//mix samples
			MIXTYPE* mixtarget = (MIXTYPE*)((char*)mixbuf + filledbytes);
			float* mixsource = (float*)mixbuf2;
			unsigned int r = 0;
			while (r < mixsamples) {
				double sourcevalue = *mixsource;
				sourcevalue = (sourcevalue + 1)/2;
				double targetvalue = *mixtarget;
				targetvalue = (targetvalue + 1)/2;

				if (bufferinitialisedsamples <= r) {
					bufferinitialisedsamples = r + 1;
					targetvalue = 0;
				}
				
				double result = -(targetvalue * sourcevalue) + (targetvalue + sourcevalue);
				result = (result * 2) - 1;
				*mixtarget = (MIXTYPE)result;
				r++;
				mixsource++;
				mixtarget++;
				filledmixfull++;
			}
		}
		i++;
	}
}

int s16mixmode = 0;

char* streambuf = NULL;
unsigned int streambuflen = 0;
void* audiomixer_GetBuffer(unsigned int len) { //SOUND THREAD
	unsigned int s16downmixlen = 0;
	if (s16mixmode) {
		len *= 2; //get twice the amount of float 32 samples
		s16downmixlen = len;
	}

	if (streambuflen != len && (streambuflen < len || streambuflen > len * 2)) {
		if (streambuf) {free(streambuf);}
		streambuf = malloc(len);
		streambuflen = len;
	}
	memset(streambuf, 0, len);

	char* p = streambuf;
	while (len > 0) {
		audiomixer_RequestMix(len);
		
		//see how much bytes we can get
		unsigned int filledbytes = filledmixpartial + filledmixfull * sizeof(MIXTYPE);
		unsigned int amount = len;
		if (amount > filledbytes) {
			amount = filledbytes;
		}
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
	}

	//FIXME: this assumes that only complete samples are fetched
	// (which isn't guaranteed)
	if (s16mixmode) {
		unsigned int i = 0;
		unsigned int i2 = 0;
		while (i + sizeof(MIXTYPE) <= s16downmixlen) {
			float fval = *((float*)((char*)streambuf+i));
			fval *= 32767;
			*((int16_t*)((char*)streambuf+i2)) = (int16_t)fval;
			i += sizeof(MIXTYPE);
			i2 += sizeof(int16_t);
		}
	}
	
	return streambuf;
}

