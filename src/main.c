
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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#ifdef WINDOWS
#include <windows.h>
#define _WINDOWS_
#endif
#ifdef ANDROID
#include <android/log.h>
#endif

int wantquit = 0;
int suppressfurthererrors = 0;
int appinbackground = 0;
static int sdlinitialised = 0;
extern int drawingallowed; //stored in luafuncs.c

#include "luastate.h"
#include "file.h"
#include "graphics.h"
#include "timefuncs.h"
#include "audio.h"
#include "main.h"
#include "audiomixer.h"
#include "logging.h"
#include "audiosourceffmpeg.h"
#include "physics.h"
#include "connections.h"
#ifdef NOTHREADEDSDLRW
#include "SDL.h"
#include "threading.h"
#endif

#define TIMESTEP 16

struct physicsworld* physicsdefaultworld = NULL;
void* main_DefaultPhysicsPtr() {
    return physicsdefaultworld;
}

void main_Quit(int returncode) {
    if (sdlinitialised) {
        //audio_Quit(); //FIXME: workaround for http://bugzilla.libsdl.org/show_bug.cgi?id=1396 (causes an unclean shutdown)
        graphics_Quit();
    }
    exit(returncode);
}

void fatalscripterror() {
    wantquit = 1;
    suppressfurthererrors = 1;
}

void printerror(const char* fmt, ...) {
    char printline[2048];
    va_list a;
    va_start(a, fmt);
    vsnprintf(printline, sizeof(printline)-1, fmt, a);
    printline[sizeof(printline)-1] = 0;
    va_end(a);
#ifdef ANDROID
    __android_log_print(ANDROID_LOG_ERROR, "blitwizard", "%s", printline);
#else
    fprintf(stderr,"%s\n",printline);
    fflush(stderr);
#endif
#ifdef WINDOWS
    //we want graphical error messages for windows
    if (!suppressfurthererrors) {
        //minimize drawing window if fullscreen
        if (graphics_AreGraphicsRunning() && graphics_IsFullscreen()) {
            graphics_MinimizeWindow();
        }
        //show error msg
        char printerror[4096];
        snprintf(printerror, sizeof(printerror)-1,
        "The application cannot continue due to a fatal error:\n\n%s",
        printline);
        MessageBox(graphics_GetWindowHWND(), printerror, "Fatal error", MB_OK|MB_ICONERROR);
    }
#endif
}

void printwarning(const char* fmt, ...) {
    char printline[2048];
    va_list a;
    va_start(a, fmt);
    vsnprintf(printline, sizeof(printline)-1, fmt, a);
    printline[sizeof(printline)-1] = 0;
    va_end(a);
#if defined(ANDROID) || defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_ERROR, "blitwizard", "%s", printline);
#else
    fprintf(stderr,"%s\n",printline);
    fflush(stderr);
#endif
}

void printinfo(const char* fmt, ...) {
    char printline[2048];
    va_list a;
    va_start(a, fmt);
    vsnprintf(printline, sizeof(printline)-1, fmt, a);
    printline[sizeof(printline)-1] = 0;
    va_end(a);
#if defined(ANDROID) || defined(__ANDROID__)
    __android_log_print(ANDROID_LOG_INFO, "blitwizard", "%s", printline);
#else
    printf("%s\n",printline);
    fflush(stdout);
#endif
}

int simulateaudio = 0;
int audioinitialised = 0;
void main_InitAudio() {
    if (audioinitialised) {return;}
    audioinitialised = 1;
    char* error;

    //get audio backend
    char* p = luastate_GetPreferredAudioBackend();

    //load FFmpeg if we happen to want it
    if (luastate_GetWantFFmpeg()) {
        audiosourceffmpeg_LoadFFmpeg();
    }else{
        audiosourceffmpeg_DisableFFmpeg();
    }

    //initialise audio - try 32bit first
    s16mixmode = 0;
#ifndef FORCES16AUDIO
    if (!audio_Init(&audiomixer_GetBuffer, 0, p, 0, &error)) {
        if (error) {free(error);}
#endif
        //try 16bit now
        s16mixmode = 1;
        if (!audio_Init(&audiomixer_GetBuffer, 0, p, 1, &error)) {
            printwarning("Warning: Failed to initialise audio: %s",error);
            if (error) {
                free(error);
            }
            //non-fatal: we will simulate audio manually:
            simulateaudio = 1;
            s16mixmode = 0;
        }
#ifndef FORCES16AUDIO
    }
#endif
    if (p) {
        free(p);
    }
}


static void quitevent() {
    char* error;
    if (!luastate_CallFunctionInMainstate("blitwiz.on_close", 0, 1, 1, &error, NULL)) {
        printerror("Error when calling blitwiz.on_close: %s",error);
        if (error) {free(error);}
        fatalscripterror();
        return;
    }
}
static void mousebuttonevent(int button, int release, int x, int y) {
    char* error;
    char onmouseup[] = "blitwiz.on_mouseup";
    const char* funcname = "blitwiz.on_mousedown";
    if (release) {
        funcname = onmouseup;
    }
    if (!luastate_PushFunctionArgumentToMainstate_Double(button)) {
        printerror("Error when pushing func args to %s",funcname);
        fatalscripterror();
        main_Quit(1);
        return;
    }
    if (!luastate_PushFunctionArgumentToMainstate_Double(x)) {
        printerror("Error when pushing func args to %s",funcname);
        fatalscripterror();
        main_Quit(1);
        return;
    }
    if (!luastate_PushFunctionArgumentToMainstate_Double(y)) {
        printerror("Error when pushing func args to %s",funcname);
        fatalscripterror();
        main_Quit(1);
        return;
    }
    if (!luastate_CallFunctionInMainstate(funcname, 3, 1, 1, &error, NULL)) {
        printerror("Error when calling %s: %s", funcname, error);
        if (error) {free(error);}
        fatalscripterror();
        main_Quit(1);
        return;
    }
}
static void mousemoveevent(int x, int y) {
    char* error;
    if (!luastate_PushFunctionArgumentToMainstate_Double(x)) {
        printerror("Error when pushing func args to blitwiz.on_mousemove");
        fatalscripterror();
        main_Quit(1);
        return;
    }
    if (!luastate_PushFunctionArgumentToMainstate_Double(y)) {
        printerror("Error when pushing func args to blitwiz.on_mousemove");
        fatalscripterror();
        main_Quit(1);
        return;
    }
    if (!luastate_CallFunctionInMainstate("blitwiz.on_mousemove", 2, 1, 1, &error, NULL)) {
        printerror("Error when calling blitwiz.on_mousemove: %s", error);
        if (error) {free(error);}
        fatalscripterror();
        main_Quit(1);
        return;
    }
}
static void keyboardevent(const char* button, int release) {
    char* error;
    char onkeyup[] = "blitwiz.on_keyup";
    const char* funcname = "blitwiz.on_keydown";
    if (release) {funcname = onkeyup;}
    if (!luastate_PushFunctionArgumentToMainstate_String(button)) {
        printerror("Error when pushing func args to %s", funcname);
        fatalscripterror();
        main_Quit(1);
        return;
    }
    if (!luastate_CallFunctionInMainstate(funcname, 1, 1, 1, &error, NULL)) {
        printerror("Error when calling %s: %s", funcname, error);
        if (error) {free(error);}
        fatalscripterror();
        main_Quit(1);
        return;
    }
}
static void textevent(const char* text) {
    char* error;
    if (!luastate_PushFunctionArgumentToMainstate_String(text)) {
        printerror("Error when pushing func args to blitwiz.on_text");
        fatalscripterror();
        return;
    }
    if (!luastate_CallFunctionInMainstate("blitwiz.on_text", 1, 1, 1, &error, NULL)) {
        printerror("Error when calling blitwiz.on_text: %s", error);
        if (error) {free(error);}
        fatalscripterror();
        return;
    }
}

static void putinbackground(int background) {
    if (background) {
        //remember we are in the background
        appinbackground = 1;
    }else{
        //restore textures and wipe old ones
#ifdef ANDROID
        graphics_ReopenForAndroid();
#endif
        //we are back in the foreground! \o/
        appinbackground = 0;
    }
}

static void imgloaded(int success, const char* texture) {
    char* error;
    if (!luastate_PushFunctionArgumentToMainstate_String(texture)) {
        printerror("Error when pushing func args to blitwiz.on_image");
        fatalscripterror();
        return;
    }
    if (!luastate_PushFunctionArgumentToMainstate_Bool(success)) {
        printerror("Error when pushing func args to blitwiz.on_image");
        fatalscripterror();
        return;
    }
    if (!luastate_CallFunctionInMainstate("blitwiz.on_image", 2, 1, 1, &error, NULL)) {
        printerror("Error when calling blitwiz.on_image: %s", error);
        if (error) {free(error);}
        fatalscripterror();
        return;
    }
}



#ifdef NOTHREADEDSDLRW
SDL_RWops* rwread_rwops = NULL;
void* rwread_buffer;
size_t rwread_readsize;
unsigned int rwread_readbytes;
int rwread_result = 0;
void* rwclose_handle = NULL;
char* rwopen_path = NULL;
void* rwopen_result = NULL;
mutex* rwread_querymutex = NULL; //only one thread may pose a query at once
mutex* rwread_executemutex = NULL; //the main thread locks this during execution of the query

void main_NoThreadedRWopsClose(void* rwops) {
    //get permission to pose a query
    mutex_Lock(rwread_querymutex);

    //make sure main thread is not doing anything
    mutex_Lock(rwread_executemutex);

    //pose query:
    rwclose_handle = rwops;

    //wait for query to be completed:
    mutex_Release(rwread_executemutex);
    while (1) {
        time_Sleep(10);
        mutex_Lock(rwread_executemutex);
        if (rwclose_handle == NULL) {break;}
        mutex_Release(rwread_executemutex);
    }

    //we are done!
    mutex_Release(rwread_executemutex);
    mutex_Release(rwread_querymutex);
}

void* main_NoThreadedRWopsOpen(const char* path) {
    //get permission to pose a query
    mutex_Lock(rwread_querymutex);

    //make sure main thread is not doing anything
    mutex_Lock(rwread_executemutex);

    //pose query:
    rwopen_path = strdup(path);
    rwopen_result = NULL;

    //wait for query to be completed:
    mutex_Release(rwread_executemutex);
    while (1) {
        time_Sleep(10);
        mutex_Lock(rwread_executemutex);
        if (rwopen_result) {break;}
        mutex_Release(rwread_executemutex);
    }

    void* handleresult = rwopen_result;
    rwopen_result = NULL;

    //we are done!
    mutex_Release(rwread_executemutex);
    mutex_Release(rwread_querymutex);
    return handleresult;
}

int main_NoThreadedRWopsRead(void* rwops, void* buffer, size_t size, unsigned int bytes) {
    //get permission to pose a query
    mutex_Lock(rwread_querymutex);

    //make sure main thread is not doing anything
    mutex_Lock(rwread_executemutex);

    //pose query:
    rwread_rwops = (SDL_RWops*)rwops;
    rwread_readsize = size;
    rwread_readbytes = bytes;
    rwread_buffer = buffer;
    rwread_result = -2;

    //wait for query to finish
    mutex_Release(rwread_executemutex);
    while (1) {
        time_Sleep(10);
        mutex_Lock(rwread_executemutex);
        if (rwread_result > -2) {break;}
        mutex_Release(rwread_executemutex);
    }
    
    //we are done!
    int r = rwread_result;
    rwread_result = 0;
    mutex_Release(rwread_executemutex);
    mutex_Release(rwread_querymutex);
    return r;
}
int main_ProcessNoThreadedReading() {
    int querydone = 0;
    mutex_Lock(rwread_executemutex);
    if (rwread_result == -2) {
        //we should read something
        rwread_result = rwread_rwops->read(rwread_rwops, rwread_buffer, rwread_readsize, rwread_readbytes);
        if (rwread_result < -1) {
            rwread_result = -1;
        }
        querydone = 1;
    }
    if (rwclose_handle) {
        //we should close something
        ((SDL_RWops*)rwclose_handle)->close(rwclose_handle);
        rwclose_handle = NULL;
    }
    if (rwopen_path) {
        //we should open something
        rwopen_result = SDL_RWFromFile(rwopen_path, "rb");
        free(rwopen_path);
        rwopen_path = NULL;
    }
    mutex_Release(rwread_executemutex);
    //make sure to wait until the query has been fully processed by
    //the requesting thread:
    mutex_Lock(rwread_querymutex);
    mutex_Release(rwread_querymutex);
    //This should improve the possibility that another thread poses
    //a new request right now and we can process it immediately.
    return querydone;
}
#endif

//with a maximum of 50 iterations, we render at least every 50 * TIMESTEP = 50 * 16 = 800 milliseconds
#define MAXLOGICITERATIONS 40

int luafuncs_ProcessNetEvents();

#if (defined(__ANDROID__) || defined(ANDROID))
int SDL_main(int argc, char** argv) {
#else
int main(int argc, char** argv) {
#endif

#ifdef NOTHREADEDSDLRW
    rwread_querymutex = mutex_Create();
    rwread_executemutex = mutex_Create();
#endif

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard %s starting", VERSION);
#endif

    //evaluate command line arguments:
    const char* script = "game.lua";
    int i = 1;
    int scriptargfound = 0;
    int option_changedir = 0;
    int gcframecount = 0;
    while (i < argc) {
        if (argv[i][0] == '-' || strcasecmp(argv[i],"/?") == 0) {
            if (strcasecmp(argv[i],"--help") == 0 || strcasecmp(argv[i], "-help") == 0
            || strcasecmp(argv[i], "-?") == 0 || strcasecmp(argv[i],"/?") == 0
            || strcasecmp(argv[i],"-h") == 0) {
                printf("blitwizard %s\n",VERSION);
                printf("Usage: blitwizard [options] [lua script]\n");
                printf("   -changedir: Change working directory to script dir\n");
                printf("   -help: Show this help text and quit\n");
                return 0;
            }
            if (strcasecmp(argv[i],"-changedir") == 0) {
                option_changedir = 1;
                i++;
                continue;
            }
            printerror("Error: Unknown option: %s",argv[i]);
            return -1;
        }else{
            if (!scriptargfound) {
                scriptargfound = 1;
                script = argv[i];
            }
        }
        i++;
    }
    
    //This needs to be done at some point before we actually initialise audio
    audiomixer_Init();

    //check the provided path:
    char outofmem[] = "Out of memory";
    char* error;
    char* filenamebuf = NULL;

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: locating lua start script...");
#endif

    //check if a folder:
    if (file_IsDirectory(script)) {
        filenamebuf = file_AddComponentToPath(script, "game.lua");
        script = filenamebuf;
    }
    
    //check if we want to change directory to the provided path:
    if (option_changedir) {
        char* p = file_GetAbsoluteDirectoryPathFromFilePath(script);
        if (!p) {
            printerror("Error: NULL returned for absolute directory");
            return -1;
        }
        char* newfilenamebuf = file_GetFileNameFromFilePath(script);
        if (!newfilenamebuf) {
            free(p);
            printerror("Error: NULL returned for file name");
            return -1;
        }
        if (filenamebuf) {free(filenamebuf);}
        filenamebuf = newfilenamebuf;
        if (!file_Cwd(p)) {
            free(filenamebuf);
            printerror("Error: Cannot cd to \"%s\"",p);
            free(p);
            return -1;
        }
        free(p);
        script = filenamebuf;
    }

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: Preparing graphics framework...");
#endif

    //initialise graphics
    if (!graphics_Init(&error)) {
        printerror("Error: Failed to initialise graphics: %s",error);
        free(error);
        fatalscripterror();
        main_Quit(1);
    }
    sdlinitialised = 1;

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: Initialising physics...");
#endif

    //initialise physics
    physicsdefaultworld = physics_CreateWorld();
    if (!physicsdefaultworld) {
        printerror("Error: Failed to initialise Box2D physics");
        fatalscripterror();
        main_Quit(1);
    }

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: Reading templates if present...");
#endif

    //run templates first if we can find them
#if !defined(ANDROID) && !defined(__ANDROID__)
    if (file_DoesFileExist("templates/init.lua")) {
#else
    //on Android, see if we can read the file:
    int exists = 0;
    SDL_RWops* rwops = SDL_RWFromFile("templates/init.lua", "rb");
    if (rwops) {
        exists = 1;
        rwops->close(rwops);
    }
    if (exists) {
#endif
        if (!luastate_DoInitialFile("templates/init.lua", &error)) {
            if (error == NULL) {
                error = outofmem;
            }
            printerror("Error: An error occured when running \"templates/init.lua\": %s", error);
            if (error != outofmem) {
                free(error);
            }
            fatalscripterror();
            main_Quit(1);
        }
    }

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: Executing lua start script...");
#endif

    //open and run provided file
    if (!luastate_DoInitialFile(script, &error)) {
        if (error == NULL) {
            error = outofmem;
        }
        printerror("Error: An error occured when running \"%s\": %s", script, error);
        if (error != outofmem) {
            free(error);
        }
        fatalscripterror();
        main_Quit(1);
    }

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: Calling blitwiz.on_init...");
#endif

    //call init
    if (!luastate_CallFunctionInMainstate("blitwiz.on_init", 0, 1, 1, &error, NULL)) {
        printerror("Error: An error occured when calling blitwiz.on_init: %s",error);
        if (error != outofmem) {    
            free(error);
        }
        fatalscripterror();
        main_Quit(1);
    }
    
    //when graphics or audio is open, run the main loop
    if (graphics_AreGraphicsRunning() || (audioinitialised && !audiomixer_NoSoundsPlaying()) || !connections_NoConnectionsOpen()) {
        int blitwizonstepworked = 0;
        int blitwizondrawworked = 0;
#if defined(ANDROID) || defined(__ANDROID__)
        printinfo("Blitwizard startup: Entering main loop...");
#endif

        //Initialise audio when it isn't
        main_InitAudio();
        
        //If we failed to initialise audio, we want to simulate it
        uint64_t simulateaudiotime = 0;
        if (simulateaudio) {
            simulateaudiotime = time_GetMilliSeconds();
        }
    
        uint64_t logictimestamp = time_GetMilliSeconds();
        uint64_t lastdrawingtime = 0;
        uint64_t physicstimestamp = time_GetMilliSeconds();
        while (!wantquit) {
            blitwizonstepworked = 0;
            blitwizondrawworked = 0;
            uint64_t time = time_GetMilliSeconds();

            //this is a hack for SDL bug http://bugzilla.libsdl.org/show_bug.cgi?id=1422
#ifdef NOTHREADEDSDLRW
            uint64_t start = time_GetMilliSeconds();
            while (main_ProcessNoThreadedReading() && start + 20 > time_GetMilliSeconds()) { }
#endif      
    
            //simulate audio
            if (simulateaudio) {
                while (simulateaudiotime < time_GetMilliSeconds()) {
                    audiomixer_GetBuffer(48 * 4 * 2);
                    simulateaudiotime += 1; // 48 * 1000 times * 4 bytes * 2 channels per second = simulated 48kHz 32bit stereo audio
                }
            }

            //limit to roughly 60 FPS
            uint64_t delta = time_GetMilliSeconds()-lastdrawingtime;
            if (delta < 15) {
                if (connections_NoConnectionsOpen()) {
                    time_Sleep(16-delta);
                    connections_SleepWait(0);
                }else{
                    connections_SleepWait(16-delta);
                }
            }
            if (!luafuncs_ProcessNetEvents()) {
                //there was an error processing the events
                exit(1);
            }

            //check and trigger all sort of input events
            graphics_CheckEvents(&quitevent, &mousebuttonevent, &mousemoveevent, &keyboardevent, &textevent, &putinbackground);

            //call the step function and advance physics
            int iterations = 0;
            while ((logictimestamp < time || physicstimestamp < time) && iterations < MAXLOGICITERATIONS) {
                if (logictimestamp < time && logictimestamp < physicstimestamp) {
                    int onstepdoesntexist = 0;
                    if (!luastate_CallFunctionInMainstate("blitwiz.on_step", 0, 1, 1, &error, &onstepdoesntexist)) {
                        printerror("Error: An error occured when calling blitwiz.on_step: %s", error);
                        if (error) {free(error);}
                        fatalscripterror();
                        main_Quit(1);
                    }else{
                        if (!onstepdoesntexist) {blitwizonstepworked = 1;}
                    }
                    logictimestamp += TIMESTEP;
                }
                if (physicstimestamp < time && physicstimestamp < logictimestamp) {
                    physics_Step(physicsdefaultworld);
                    physicstimestamp += physics_GetStepSize(physicsdefaultworld);
                }
                iterations++;
            }
            if (iterations >= MAXLOGICITERATIONS) {
                physicstimestamp = time_GetMilliSeconds();
                logictimestamp = time_GetMilliSeconds();
                printwarning("Warning: logic is too slow, maximum logic iterations have been reached (%d)", (int)MAXLOGICITERATIONS);
            }

            //check for image loading progress
            if (!appinbackground) {
                graphics_CheckTextureLoading(&imgloaded);
            }
        
            if (!appinbackground) { 
                //start drawing
                drawingallowed = 1;
                graphics_StartFrame();
                
                //call the drawing function
                int ondrawdoesntexist = 0;
                if (!luastate_CallFunctionInMainstate("blitwiz.on_draw", 0, 1, 1, &error, &ondrawdoesntexist)) {
                    printerror("Error: An error occured when calling blitwiz.on_draw: %s",error);
                    if (error) {free(error);}
                    fatalscripterror();
                    main_Quit(1);
                }else{
                    if (!ondrawdoesntexist) {blitwizondrawworked = 1;}
                }
                
                //complete the drawing
                drawingallowed = 0;
                graphics_CompleteFrame();
            }else{
                blitwizondrawworked = 1;
            }

            //we might want to quit if there is nothing else to do
            if (!blitwizondrawworked && !blitwizonstepworked && connections_NoConnectionsOpen() && audiomixer_NoSoundsPlaying()) {
                main_Quit(1);
            }

            //be very sleepy if in background
            if (appinbackground) {
#ifndef NOTHREADEDSDLRW
                time_Sleep(100);
#else
                time_Sleep(20);
#endif
            }

            //do some garbage collection:
            gcframecount++;
            if (gcframecount > 100) {
                //do a gc step once in a while
                luastate_GCCollect();
            }
        }
    }
    main_Quit(0);
    return 0;
}

