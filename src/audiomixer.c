
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
	
	audiosourcefadepanvol_SetPanVol(channels[slot].fadepanvolsource, panning, volume);
	if (fadeinseconds > 0) {
		audiosourcefadepanvol_StartFade(channels[slot].fadepanvolsource, fadeinseconds, volume, 0);
	}
	
	channels[slot].loopsource = audiosourceloop_Create(channels[slot].fadepanvolsource);
	
	audiosourceloop_SetLooping(channels[slot].loopsource, loop);
	
	audio_UnlockAudioThread();
	return id;
}

static void audiomixer_FillMixAudioBuffer() { //SOUND THREAD
	
}

char* streambuf = NULL;
unsigned int streambuflen = 0;
void* audiomixer_GetBuffer(unsigned int len) { //SOUND THREAD
	if (streambuflen != len) {
		if (streambuf) {free(streambuf);}
		streambuf = malloc(len);
		streambuflen = len;
	}
	memset(streambuf, 0, len);
	
	return streambuf;
}

