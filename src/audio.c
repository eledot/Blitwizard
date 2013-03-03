
/* blitwizard game engine - source code file

  Copyright (C) 2011-2013 Jonas Thiem

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
#include "logging.h"

// valid sound buffer sizes for audio:
#define DEFAULTSOUNDBUFFERSIZE (2048)
#define MINSOUNDBUFFERSIZE 512
#define MAXSOUNDBUFFERSIZE (1024 * 10)

// since waveout is shit, we'll need a bigger buffer for it:
#define WAVEOUTMINBUFFERSIZE (2048)

#ifdef USE_AUDIO
#ifdef USE_SDL_AUDIO

#include "SDL.h"

static void (*samplecallbackptr)(void*, unsigned int) = NULL;
static int soundenabled = 0;

void audiocallback(void *intentionally_unused, Uint8 *stream, int len) {
    samplecallbackptr(stream, (unsigned int)len);
}

const char* audio_GetCurrentBackendName() {
    if (!soundenabled) {
        return NULL;
    }
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
int audio_Init(void (*samplecallback)(void*, unsigned int),
unsigned int buffersize, const char* backend, int s16, char** error) {
#ifndef USE_SDL_GRAPHICS
    if (!sdlvideoinit) {
        if (SDL_VideoInit(NULL) < 0) {
            char errormsg[512];
            snprintf(errormsg,sizeof(errormsg), "Failed to initialize SDL video: %s", SDL_GetError());
            errormsg[sizeof(errormsg)-1] = 0;
            *error = strdup(errormsg);
            return 0;
        }
        sdlvideoinit = 1;
    }
#endif
    if (soundenabled) {
        // quit old sound first
        SDL_PauseAudio(1);
        SDL_AudioQuit();
        soundenabled = 0;
    }
#ifdef ANDROID
    if (!s16) {
        *error = strdup("No 32bit float audio available on Android");
        return 0;
    }
#endif
    if (!samplecallback) {
        *error = strdup("Need sample callback");
        return 0;
    }
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
    if (strlen(b) <= 0) {
        b = NULL;
    }
    if (SDL_AudioInit(b) < 0) {
        snprintf(errbuf,sizeof(errbuf),"Failed to initialize SDL audio: %s", SDL_GetError());
        errbuf[sizeof(errbuf)-1] = 0;
        *error = strdup(errbuf);
        return 0;
    }

    SDL_AudioSpec fmt,actualfmt;

    int custombuffersize = DEFAULTSOUNDBUFFERSIZE;
    if (buffersize > 0) {
        if (buffersize < MINSOUNDBUFFERSIZE) {
            buffersize = MINSOUNDBUFFERSIZE;
        }
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
        // FIXME: this is a workaround for http:// bugzilla.libsdl.org/show_bug.cgi?id=1343 (will cause a memory leak!)
        // SDL_AudioQuit();
        return 0;
    }

    if (actualfmt.channels != 2 || actualfmt.freq != 48000 || (s16 && actualfmt.format != AUDIO_S16) || (!s16 && actualfmt.format != AUDIO_F32SYS)) {
        *error = strdup("SDL audio delivered wrong/unusable format");
        // FIXME: this is a workaround for http:// bugzilla.libsdl.org/show_bug.cgi?id=1343 (will cause a memory leak!)
        // SDL_AudioQuit();
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

#else // USE_SDL_AUDIO

#ifdef WINDOWS

// waveout audio.
// waveout is a pretty stupid api,
// but it will be sufficient to get a bit of sound out.
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>

#include "threading.h"
#include "timefuncs.h"

HWAVEOUT waveoutdev;
WAVEFORMATEX waveoutfmt;
mutex* waveoutlock = NULL;
semaphore* newblocksignal = NULL;  // semaphore is "post"'ed to signal that a new block shall be pushed
volatile int threadcontrol = -1;  // 1 - thread runs, 0 - shutdown signal, -1 thread is off
volatile int waveoutstate = -1;  // -1 not running, 0 starting, 1 running
int waveoutbytes;

// this is used by the sound thread;
#define AUDIOBLOCKS 4
WAVEHDR waveheader[AUDIOBLOCKS];
char* blockbuffer[AUDIOBLOCKS];
volatile int headerprepared[AUDIOBLOCKS];
int nextaudioblock = 0;

// audio mixer callback and global sound enabled info:
static void (*samplecallbackptr)(void*, unsigned int) = NULL;
static int soundenabled = 0;

const char* audio_GetCurrentBackendName(void) {
    return "waveout";
}

static void queueBlock(void) {
    // output new audio (SOUND THREAD)
    mutex_Lock(waveoutlock);

    // find out which block we want to queue up:
    int i = nextaudioblock;
    nextaudioblock++;
    if (nextaudioblock >= AUDIOBLOCKS) {
        nextaudioblock = 0;
    }
    int previousaudioblock = i-(AUDIOBLOCKS-1);
    while (previousaudioblock < 0) {
        previousaudioblock += AUDIOBLOCKS;
    }

    // set block length
    waveheader[i].dwBufferLength = waveoutbytes;
    waveheader[i].dwLoops = 0;

    // set block audio data:
    samplecallbackptr(blockbuffer[i], (unsigned int)waveoutbytes);
    waveheader[i].lpData = blockbuffer[i];

    // queue up block:
    int r;
    if (!headerprepared[i]) {
        if ((r = waveOutPrepareHeader(waveoutdev, &waveheader[i],
        sizeof(WAVEHDR))) != MMSYSERR_NOERROR) {
            printf("prepare failed! %d\n", r);fflush(stdout);
            mutex_Release(waveoutlock);
            return;
        }
        headerprepared[i] = 1;
    }

    if ((r = waveOutWrite(waveoutdev, &waveheader[i],
    sizeof(WAVEHDR))) != MMSYSERR_NOERROR) {
        printf("writeout failed!\n");fflush(stdout);
        mutex_Release(waveoutlock);
        return;
    }
    fflush(stdout);

    mutex_Release(waveoutlock);
}

static void CALLBACK audioCallback(HWAVEOUT hwo, UINT uMsg,
DWORD dwInstance, DWORD dwParam1, DWORD dwParam2) {
    if (uMsg == WOM_DONE) {
        queueBlock();
        // semaphore_Post(newblocksignal);
    }
}

void audio_SoundThread(void* userdata) {
    // sound thread function (SOUND THREAD)

    // initialise audio blocks:
    int i = 0;
    while (i < AUDIOBLOCKS) {
        // keep in mind we still need to waveout-prepare this block:
        headerprepared[i] = 0;

        // zero out the block:
        memset(&waveheader[i], 0, sizeof(WAVEHDR));

        // initialise new block data:
        blockbuffer[i] = malloc(waveoutbytes);
        i++;
    }

    // initialise signal semaphore:
    if (newblocksignal) {
        semaphore_Destroy(newblocksignal);
    }
    newblocksignal = semaphore_Create(0);

    // open audio device
    MMRESULT r = waveOutOpen(&waveoutdev, WAVE_MAPPER,
    &waveoutfmt, (DWORD_PTR)&audioCallback, (DWORD_PTR)0,
    (DWORD)CALLBACK_FUNCTION | WAVE_ALLOWSYNC);
    mutex_Lock(waveoutlock);
    if (r != MMSYSERR_NOERROR) {
        threadcontrol = -1;
        mutex_Release(waveoutlock);
        return;
    }
    threadcontrol = 1;

    mutex_Release(waveoutlock);

    // queue up the initial blocks:
    i = 0;
    while (i < AUDIOBLOCKS) {
        queueBlock();
        i++;
    }

    while (1) {
        // the waveOut callback will wake us up when there
        // is stuff to do:
        // semaphore_Wait(newblocksignal);
        // queueBlock();
        time_Sleep(100);
        mutex_Lock(waveoutlock);
        if (threadcontrol == 0) {
            // we are supposed to shutdown
            // clean up all buffers
            int i = 0;
            while (i < AUDIOBLOCKS) {
                if (headerprepared[i]) {
                    if (waveOutUnprepareHeader(
                    waveoutdev, &waveheader[i],
                    sizeof(WAVEHDR)) != MMSYSERR_NOERROR) {
                        // buffer might be still playing, sleep a bit:
                        time_Sleep(100);
                    }
                    headerprepared[i] = 0;
                    // delete buffer contents:
                    free(blockbuffer[i]);
                    blockbuffer[i] = NULL;
                }
                i++;
            }
            // close device:
            waveOutClose(waveoutdev);
            waveoutdev = NULL;
            // tell main thread we're done:
            threadcontrol = -1;
            mutex_Release(waveoutlock);
            return;
        }
        mutex_Release(waveoutlock);
    }
}

static void audio_StopWaveoutThread(void) {
    mutex_Lock(waveoutlock);
    if (threadcontrol <= 0) {
        if (threadcontrol == 0) {
            // wait for thread to shut down:
            while (threadcontrol == 0) {
                mutex_Release(waveoutlock);
                time_Sleep(50);
                mutex_Lock(waveoutlock);
            }
        }
        mutex_Release(waveoutlock);
        return;
    }

    // tell thread to shutdown:
    threadcontrol = 0;

    // wait for thread to shutdown:
    while (threadcontrol == 0) {
        mutex_Release(waveoutlock);
        time_Sleep(50);
        mutex_Lock(waveoutlock);
    }
    mutex_Release(waveoutlock);
}

static void waveout_LaunchWaveoutThread(void) {
    mutex_Lock(waveoutlock);
    if (threadcontrol >= 0) {
        // thread is already running
        mutex_Release(waveoutlock);
        return;
    }
    threadcontrol = 0;
    mutex_Release(waveoutlock);

    // launch our sound thread which will operate the waveOut device:
    threadinfo* t = thread_CreateInfo();
    if (!t) {
        return;
    }
    thread_Spawn(t, audio_SoundThread, NULL);
    thread_FreeInfo(t);
}

void audio_Quit(void) {
    if (soundenabled) {
        audio_StopWaveoutThread();
        soundenabled = 0;
    }
}

int audio_Init(void (*samplecallback)(void*, unsigned int),
unsigned int buffersize, const char* backend, int s16, char** error) {
    if (!s16) {
        *error = strdup("WaveOut doesn't support 32bit float audio");
        return 0;
    }

    if (!waveoutlock) {
        waveoutlock = mutex_Create();
    }

    if (soundenabled) {
        // quit old sound first
        audio_StopWaveoutThread();
        soundenabled = 0;
    }

    if (!samplecallback) {
        *error = strdup("Need sample callback");
        return 0;
    }

    int i = 0;
    while (i < AUDIOBLOCKS) {
        blockbuffer[i] = NULL;
        i++;
    }

    memset(&waveoutfmt, 0, sizeof(waveoutfmt));
    waveoutfmt.nSamplesPerSec = 48000;
    waveoutfmt.wBitsPerSample = 16;
    waveoutfmt.nChannels = 2;

    waveoutfmt.cbSize = 0;
    waveoutfmt.wFormatTag = WAVE_FORMAT_PCM;
    waveoutfmt.nBlockAlign = (2 * 16) / 8;
    waveoutfmt.nAvgBytesPerSec = waveoutfmt.nSamplesPerSec * waveoutfmt.nBlockAlign;

    int custombuffersize = DEFAULTSOUNDBUFFERSIZE;
    if (buffersize > 0) {
        custombuffersize = buffersize;
    }
    if (custombuffersize < WAVEOUTMINBUFFERSIZE) {
        custombuffersize = WAVEOUTMINBUFFERSIZE;
    }
    if (custombuffersize > MAXSOUNDBUFFERSIZE) {
        custombuffersize = MAXSOUNDBUFFERSIZE;
    }
    waveoutbytes = custombuffersize;
    samplecallbackptr = samplecallback;
    waveout_LaunchWaveoutThread();

    time_Sleep(50);
    mutex_Lock(waveoutlock);
    while (threadcontrol == 0) {
        mutex_Release(waveoutlock);
        time_Sleep(50);
        mutex_Lock(waveoutlock);
    }
    mutex_Release(waveoutlock);

    if (threadcontrol < 0) {
        *error = strdup("WaveOut returned an error");
        return 0;
    }
    return 1;
}

#else  // WINDOWS

// no audio support

const char* audio_GetCurrentBackendName() {
    return NULL;
}

#endif  // WINDOWS
#endif  // USE_SDL_AUDIO
#endif  // USE_AUDIO

