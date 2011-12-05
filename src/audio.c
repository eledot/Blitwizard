
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

#include "SDL.h"

#define DEFAULTSOUNDBUFFERSIZE 1024
#define MINSOUNDBUFFERSIZE 512
#define MAXSOUNDBUFFERSIZE (1024 * 10)

static void*(*samplecallbackptr)(unsigned int) = NULL;

void audiocallback(void *intentionally_unused, Uint8 *stream, int len) {
	memset(stream, 0, (unsigned int)len);
	if (!samplecallbackptr) {return;}
	SDL_MixAudio(stream, samplecallbackptr((unsigned int)len), (int)len, SDL_MIX_MAXVOLUME);
	//memcpy(stream, samplecallbackptr((unsigned int)len), (unsigned int)len);
}

int audio_Init(void*(*samplecallback)(unsigned int), unsigned int buffersize, const char* backend, char** error) {
	char errbuf[512];
	char preferredbackend[20] = "";
#ifdef WIN
	if (backend && strcasecmp(backend, "waveout") == 0) {
		strcpy(preferredbackend, "waveout");
	}
#endif
	const char* b = preferredbackend;
	if (strlen(b) <= 0) {b = NULL;}
	if (SDL_AudioInit(b) < 0) {
		snprintf(errbuf,sizeof(errbuf),"Failed to initialize SDL audio: %s", SDL_GetError());
		errbuf[sizeof(errbuf)-1] = 0;
		*error = strdup(errbuf);
		return 0;
	}
	
	SDL_AudioSpec fmt;
	
	int custombuffersize = DEFAULTSOUNDBUFFERSIZE;
	if (buffersize > 0) {
		if (buffersize < MINSOUNDBUFFERSIZE) {buffersize = MINSOUNDBUFFERSIZE;}
		if (buffersize > MAXSOUNDBUFFERSIZE) {
			buffersize = MAXSOUNDBUFFERSIZE;
		}
		custombuffersize = buffersize;
	}

	fmt.freq = 48000;
	fmt.format = AUDIO_S16;
	fmt.channels = 2;
	fmt.samples = custombuffersize;
	fmt.callback = audiocallback;
	fmt.userdata = NULL;
	
	samplecallbackptr = samplecallback;
		
	if (SDL_OpenAudio(&fmt, NULL) < 0) {
		snprintf(errbuf,sizeof(errbuf),"Failed to open SDL audio: %s", SDL_GetError());
		errbuf[sizeof(errbuf)-1] = 0;
		*error = strdup(errbuf);
		return 0;
	}
	
	SDL_PauseAudio(0);
	return 1;
}

void audio_LockAudioThread() {
	SDL_LockAudio();
}

void audio_UnlockAudioThread() {
	SDL_UnlockAudio();
}

