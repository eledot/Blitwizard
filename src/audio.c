
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

#include "os.h"
#include <stdint.h>
#include <stdlib.h>

#ifdef USE_AUDIO
#ifdef USE_SDL_AUDIO

#include "SDL.h"

#ifndef NOTHREADEDSDLRW
#define DEFAULTSOUNDBUFFERSIZE 2048
#else
#define DEFAULTSOUNDBUFFERSIZE 4096
#endif
#define MINSOUNDBUFFERSIZE 512
#define MAXSOUNDBUFFERSIZE (1024 * 10)

static void*(*samplecallbackptr)(unsigned int) = NULL;
static int soundenabled = 0;

void audiocallback(void *intentionally_unused, Uint8 *stream, int len) {
    memset(stream, 0, (size_t)len);
    if (!samplecallbackptr) {return;}
    SDL_MixAudio(stream, samplecallbackptr((unsigned int)len), (unsigned int)len, SDL_MIX_MAXVOLUME);
    //memcpy(stream, samplecallbackptr((unsigned int)len), (unsigned int)len);
}

const char* audio_GetCurrentBackendName() {
    if (!soundenabled) {return NULL;}
    return SDL_GetCurrentAudioDriver();
}


void audio_Quit() {
    if (soundenabled) {
        SDL_CloseAudio();
        SDL_AudioQuit();
        soundenabled = 0;
    }
}

#ifndef USE_SDL_GRAPHICS
static int sdlvideoinit = 1;
#endif
int audio_Init(void*(*samplecallback)(unsigned int), unsigned int buffersize, const char* backend, int s16, char** error) {
#ifndef USE_SDL_GRAPHICS
    if (!sdlvideoinit) {
        if (SDL_VideoInit(NULL) < 0) {
            snprintf(errormsg,sizeof(errormsg),"Failed to initialize SDL video: %s", SDL_GetError());
            errormsg[sizeof(errormsg)-1] = 0;
            *error = strdup(errormsg);
            return 0;
        }
        sdlvideoinit = 1;
    }
#endif
    if (soundenabled) {
        //quit old sound first
        SDL_PauseAudio(1);
        SDL_AudioQuit();
        soundenabled = 0;
    }
#ifdef ANDROID
    if (!s16) {
        *error = strdup("No 32bit audio available on Android");
        return 0;
    }
#endif
    char errbuf[512];
    char preferredbackend[20] = "";
#ifdef WINDOWS
    if (backend && strcasecmp(backend, "waveout") == 0) {
        strcpy(preferredbackend, "waveout");
    }
    if (backend && (strcasecmp(backend, "directsound") == 0 || strcasecmp(backend, "dsound") == 0)) {
        strcpy(preferredbackend, "directsound");
    }
#else
#ifdef LINUX
    if (backend && strcasecmp(backend, "alsa") == 0) {
        strcpy(preferredbackend, "alsa");
    }
    if (backend && (strcasecmp(backend, "oss") == 0 || strcasecmp(backend, "dsp") == 0)) {
        strcpy(preferredbackend, "dsp");
    }
#endif
#endif
    const char* b = preferredbackend;
    if (strlen(b) <= 0) {b = NULL;}
    if (SDL_AudioInit(b) < 0) {
        snprintf(errbuf,sizeof(errbuf),"Failed to initialize SDL audio: %s", SDL_GetError());
        errbuf[sizeof(errbuf)-1] = 0;
        *error = strdup(errbuf);
        return 0;
    }
    
    SDL_AudioSpec fmt,actualfmt;
    
    int custombuffersize = DEFAULTSOUNDBUFFERSIZE;
    if (buffersize > 0) {
        if (buffersize < MINSOUNDBUFFERSIZE) {buffersize = MINSOUNDBUFFERSIZE;}
        if (buffersize > MAXSOUNDBUFFERSIZE) {
            buffersize = MAXSOUNDBUFFERSIZE;
        }
        custombuffersize = buffersize;
    }

    memset(&fmt,0,sizeof(fmt));
    fmt.freq = 48000;
    if (!s16) {
        fmt.format = AUDIO_F32SYS;
    }else{
        fmt.format = AUDIO_S16;
    }
    fmt.channels = 2;
    fmt.samples = custombuffersize;
    fmt.callback = audiocallback;
    fmt.userdata = NULL;
    
    samplecallbackptr = samplecallback;
        
    if (SDL_OpenAudio(&fmt, &actualfmt) < 0) {
        snprintf(errbuf,sizeof(errbuf),"Failed to open SDL audio: %s", SDL_GetError());
        errbuf[sizeof(errbuf)-1] = 0;
        *error = strdup(errbuf);
        //FIXME: this is a workaround for http://bugzilla.libsdl.org/show_bug.cgi?id=1343 (will cause a memory leak!)
        //SDL_AudioQuit();
        return 0;
    }

    if (actualfmt.channels != 2 || actualfmt.freq != 48000 || (s16 && actualfmt.format != AUDIO_S16) || (!s16 && actualfmt.format != AUDIO_F32SYS)) {
        *error = strdup("SDL audio delivered wrong/unusable format");
        //FIXME: this is a workaround for http://bugzilla.libsdl.org/show_bug.cgi?id=1343 (will cause a memory leak!)
        //SDL_AudioQuit();
        return 0;
    }
    
    soundenabled = 1;
    SDL_PauseAudio(0);
    return 1;
}

void audio_LockAudioThread() {
    SDL_LockAudio();
}

void audio_UnlockAudioThread() {
    SDL_UnlockAudio();
}

#else //USE_SDL_AUDIO

const char* audio_GetCurrentBackendName() {
    return NULL;
}

#endif //USE_SDL_AUDIO
#endif //USE_AUDIO

