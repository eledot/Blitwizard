
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
#include <inttypes.h>
#ifdef WINDOWS
#include <windows.h>
#define _WINDOWS_
#endif
#ifdef ANDROID
#include <android/log.h>
#endif

// physics2d callback we will need later when setting up the physics simulation
struct physicsobject2d;
int luafuncs_globalcollision2dcallback_unprotected(void* userdata, struct physicsobject2d* a, struct physicsobject2d* b, double x, double y, double normalx, double normaly, double force);

int wantquit = 0; // set to 1 if there was a quit event
int suppressfurthererrors = 0; // a critical error was shown, don't show more
int windowisfocussed = 0;
int appinbackground = 0; // app is in background (mobile/Android)
static int sdlinitialised = 0; // sdl was initialised and needs to be quit
extern int drawingallowed; // stored in luafuncs.c, checks if we come from an on_draw() event or not

#include "luastate.h"
#include "file.h"
#include "timefuncs.h"
#include "audio.h"
#include "main.h"
#include "audiomixer.h"
#include "logging.h"
#include "audiosourceffmpeg.h"
#include "physics.h"
#include "connections.h"
#include "listeners.h"
#ifdef USE_SDL_GRAPHICS
#include "SDL.h"
#endif
#include "graphicstexture.h"
#include "graphics.h"

int TIMESTEP = 16;
int MAXLOGICITERATIONS = 50; // 50 * 16 = 800ms

void main_SetTimestep(int timestep) {
    if (timestep < 16) {
        timestep = 16;
    }
    TIMESTEP = timestep;
    MAXLOGICITERATIONS = 800 / timestep; // never do logic for longer than 800ms
    if (MAXLOGICITERATIONS < 2) {
        MAXLOGICITERATIONS = 2;
    }
}

struct physicsworld2d* physics2ddefaultworld = NULL;
void* main_DefaultPhysics2dPtr() {
    return physics2ddefaultworld;
}

void main_Quit(int returncode) {
    listeners_CloseAll();
    if (sdlinitialised) {
#ifdef USE_SDL_AUDIO
        // audio_Quit(); // FIXME: workaround for http://bugzilla.libsdl.org/show_bug.cgi?id=1396 (causes an unclean shutdown)
#endif
#ifdef USE_GRAPHICS
        graphics_Quit();
#endif
    }
    exit(returncode);
}

void fatalscripterror(void) {
    wantquit = 1;
    suppressfurthererrors = 1;
}

int simulateaudio = 0;
int audioinitialised = 0;
void main_InitAudio(void) {
#ifdef USE_AUDIO
    if (audioinitialised) {
        return;
    }
    audioinitialised = 1;

    // get audio backend
#ifdef USE_SDL_AUDIO
    char* p = luastate_GetPreferredAudioBackend();
#endif

    // load FFmpeg if we happen to want it
    if (luastate_GetWantFFmpeg()) {
        audiosourceffmpeg_LoadFFmpeg();
    }else{
        audiosourceffmpeg_DisableFFmpeg();
    }

#ifdef USE_SDL_AUDIO
    char* error;

    // initialise audio - try 32bit first
    s16mixmode = 0;
#ifndef FORCES16AUDIO
    if (!audio_Init(&audiomixer_GetBuffer, 0, p, 0, &error)) {
        if (error) {
            free(error);
        }
#endif
        // try 16bit now
        s16mixmode = 1;
        if (!audio_Init(&audiomixer_GetBuffer, 0, p, 1, &error)) {
            printwarning("Warning: Failed to initialise audio: %s",error);
            if (error) {
                free(error);
            }
            // non-fatal: we will simulate audio manually:
            simulateaudio = 1;
            s16mixmode = 0;
        }
#ifndef FORCES16AUDIO
    }
#endif
    if (p) {
        free(p);
    }
#else
    // simulate audio:
    simulateaudio = 1;
    s16mixmode = 0;
#endif
#else // ifdef USE_AUDIO
    return;
#endif // ifdef USE_AUDIO
}


static void quitevent() {
    char* error;
    if (!luastate_CallFunctionInMainstate("blitwiz.on_close", 0, 1, 1, &error, NULL)) {
        printerror("Error when calling blitwiz.on_close: %s",error);
        if (error) {
            free(error);
        }
        fatalscripterror();
        return;
    }
}

void imgloadedcallback(int success, const char* texture) {
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
        if (error) {
            free(error);
        }
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
        if (error) {
            free(error);
        }
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
        if (error) {
            free(error);
        }
        fatalscripterror();
        main_Quit(1);
        return;
    }
}
static void keyboardevent(const char* button, int release) {
    char* error;
    char onkeyup[] = "blitwiz.on_keyup";
    const char* funcname = "blitwiz.on_keydown";
    if (release) {
        funcname = onkeyup;
    }
    if (!luastate_PushFunctionArgumentToMainstate_String(button)) {
        printerror("Error when pushing func args to %s", funcname);
        fatalscripterror();
        main_Quit(1);
        return;
    }
    if (!luastate_CallFunctionInMainstate(funcname, 1, 1, 1, &error, NULL)) {
        printerror("Error when calling %s: %s", funcname, error);
        if (error) {
            free(error);
        }
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
        if (error) {
            free(error);
        }
        fatalscripterror();
        return;
    }
}

static void putinbackground(int background) {
    if (background) {
        // remember we are in the background
        appinbackground = 1;
    }else{
        // restore textures and wipe old ones
#ifdef ANDROID
        graphics_ReopenForAndroid();
#endif
        // we are back in the foreground! \o/
        appinbackground = 0;
    }
}

void attemptTemplateLoad(const char* path) {
    char outofmem[] = "Out of memory";
    char* error;
    if (!luastate_DoInitialFile(path, 0, &error)) {
        if (error == NULL) {
            error = outofmem;
        }
        printerror("Error: An error occured when running templates init.lua: %s", error);
        if (error != outofmem) {
            free(error);
        }
        fatalscripterror();
        main_Quit(1);
        return;
    }
}


int luafuncs_ProcessNetEvents(void);

#define MAXSCRIPTARGS 1024

#if (defined(__ANDROID__) || defined(ANDROID))
int SDL_main(int argc, char** argv) {
#else
#ifdef WINDOWS
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
#else
int main(int argc, char** argv) {
#endif
#endif

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard %s starting", VERSION);
#endif

    // evaluate command line arguments:
    const char* script = "game.lua";
    int scriptargfound = 0;
    int option_changedir = 0;
    char* option_templatepath = NULL;
    int option_templatepathset = 0;
    int nextoptionistemplatepath = 0;
    int gcframecount = 0;

#ifdef WINDOWS
    // obtain command line arguments a special way on windows:
    int argc = __argc;
    char** argv = __argv;
#endif

    // we want to store the script arguments so we can pass them to lua:
    char** scriptargs = malloc(sizeof(char*) * MAXSCRIPTARGS);
    if (!scriptargs) {
        printerror("Error: failed to allocate script args space");
        return 1;
    }
    int scriptargcount = 0;

    // parse command line arguments:
    int i = 1;
    while (i < argc) {
        if (!scriptargfound) { // pre-scriptname arguments
            // process template path option parameter:
            if (nextoptionistemplatepath) {
                nextoptionistemplatepath = 0;
                option_templatepath = strdup(argv[i]);
                if (!option_templatepath) {
                    printerror("Error: failed to strdup() template path argument");
                    main_Quit(1);
                    return 1;
                }
                option_templatepathset = 1;
                file_MakeSlashesNative(option_templatepath);
                i++;
                continue;
            }

            // various options:
            if (argv[i][0] == '-' || strcasecmp(argv[i],"/?") == 0) {
                if (strcasecmp(argv[i],"--help") == 0 || strcasecmp(argv[i], "-help") == 0
                || strcasecmp(argv[i], "-?") == 0 || strcasecmp(argv[i],"/?") == 0
                || strcasecmp(argv[i],"-h") == 0) {
                    printf("blitwizard %s\n",VERSION);
                    printf("Usage: blitwizard [blitwizard options] [lua script] [script options]\n\n");
                    printf("The lua script should be a .lua file containing Lua source code.\n\n");
                    printf("The script options (optional) are passed through to the script.\n\n");
                    printf("The blitwizard options (optional) can be some of those:\n");
                    printf("   -changedir             Change working directory to the dir of the lua script path\n");
                    printf("   -help                  Show this help text and quit\n");
                    printf("   -templatepath [path]   Check another place for templates, not \"templates/\"\n");
                    return 0;
                }
                if (strcasecmp(argv[i],"-changedir") == 0) {
                    option_changedir = 1;
                    i++;
                    continue;
                }
                if (strcasecmp(argv[i],"-templatepath") == 0) {
                    nextoptionistemplatepath = 1;
                    i++;
                    continue;
                }
                printwarning("Warning: Unknown Blitwizard option: %s", argv[i]);
            }else{
                scriptargfound = 1;
                script = argv[i];
            }
        }else{
            // post-scriptname arguments -> store them for Lua
            if (scriptargcount < MAXSCRIPTARGS) {
                scriptargs[scriptargcount] = strdup(argv[i]);
                scriptargcount++;
            }
        }
        i++;
    }

#ifdef USE_AUDIO
    // This needs to be done at some point before we actually initialise audio
    audiomixer_Init();
#endif

    // check the provided path:
    char outofmem[] = "Out of memory";
    char* error;
    char* filenamebuf = NULL;

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: locating lua start script...");
#endif

    // if no template path was provided, default to "templates/"
    if (!option_templatepath) {
        option_templatepath = strdup("templates/");
        if (!option_templatepath) {
            printerror("Error: failed to allocate initial template path");
            main_Quit(1);
            return 1;
        }
        file_MakeSlashesNative(option_templatepath);
    }

    // check if provided script path is a folder:
    if (file_IsDirectory(script)) {
        filenamebuf = file_AddComponentToPath(script, "game.lua");
        if (!filenamebuf) {
            printerror("Error: failed to add component to template path");
            main_Quit(1);
            return 1;
        }
        script = filenamebuf;
    }

    // check if we want to change directory to the provided script path:
    if (option_changedir) {
        char* p = file_GetAbsoluteDirectoryPathFromFilePath(script);
        if (!p) {
            printerror("Error: NULL returned for absolute directory");
            main_Quit(1);
            return 1;
        }
        char* newfilenamebuf = file_GetFileNameFromFilePath(script);
        if (!newfilenamebuf) {
            free(p);
            printerror("Error: NULL returned for file name");
            main_Quit(1);
            return 1;
        }
        if (filenamebuf) {free(filenamebuf);}
        filenamebuf = newfilenamebuf;
        if (!file_Cwd(p)) {
            free(filenamebuf);
            printerror("Error: Cannot cd to \"%s\"",p);
            free(p);
            main_Quit(1);
            return 1;
        }
        free(p);
        script = filenamebuf;
    }

/*#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: Preparing graphics framework...");
#endif

    // initialise graphics
#ifdef USE_GRAPHICS
    if (!graphics_Init(&error)) {
        printerror("Error: Failed to initialise graphics: %s",error);
        free(error);
        fatalscripterror();
        main_Quit(1);
        return 1;
    }
    sdlinitialised = 1;
#endif*/

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: Initialising physics...");
#endif

#ifdef USE_PHYSICS2D
    // initialise physics
    physics2ddefaultworld = physics2d_CreateWorld();
    if (!physics2ddefaultworld) {
        printerror("Error: Failed to initialise Box2D physics");
        fatalscripterror();
        main_Quit(1);
        return 1;
    }
    physics2d_SetCollisionCallback(physics2ddefaultworld, &luafuncs_globalcollision2dcallback_unprotected, NULL);
#endif

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: Reading templates if present...");
#endif

    // Search & run templates. Separate code for desktop/android due to
    // android having the templates in embedded resources (where cwd'ing to
    // isn't supported), while for the desktop it is a regular folder.
#if !defined(ANDROID)
    // remember current directory:
    char* currentworkingdir = file_GetCwd();
    if (!currentworkingdir) {
        printerror("Error: failed to change current working directory");
        main_Quit(1);
        return 1;
    }

    int checksystemwidetemplate = 1;
    // see if there is a template directory & file:
    if (file_DoesFileExist(option_templatepath) && file_IsDirectory(option_templatepath)) {
        checksystemwidetemplate = 0;

        // change working directory to template folder:
        int cwdfailed = 0;
        if (!file_Cwd(option_templatepath) && option_templatepathset) {
            printwarning("Warning: failed to change working directory to template path \"%s\"", option_templatepath);
            cwdfailed = 1;
        }

        // now run template file:
        if (!cwdfailed && file_DoesFileExist("init.lua")) {
            attemptTemplateLoad("init.lua");
        }else{
            checksystemwidetemplate = 1;
        }
    }
#if defined(SYSTEM_TEMPLATE_PATH)
    if (checksystemwidetemplate) {
        if (file_Cwd(SYSTEM_TEMPLATE_PATH)) {
            if (file_DoesFileExist("init.lua")) {
                attemptTemplateLoad("init.lua");
            }
        }
    }
#endif
#else // if !defined(ANDROID)
    // on Android, we only allow templates/init.lua.
    // see if we can read the file:
    int exists = 0;
    SDL_RWops* rwops = SDL_RWFromFile("templates/init.lua", "rb");
    if (rwops) {
        exists = 1;
        rwops->close(rwops);
    }
    if (exists) {
        // run the template file:
        if (!luastate_DoInitialFile("templates/init.lua", 0, &error)) {
            attemptTemplateLoad("templates/init.lua");
        }
    }
#endif

    // now since templates are loaded, return to old working dir:
#if !defined(ANDROID)
    file_Cwd(currentworkingdir);
#endif

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: Executing lua start script...");
#endif

    // push command line arguments into script state:
    i = 0;
    int pushfailure = 0;
    while (i < scriptargcount) {
        if (!luastate_PushFunctionArgumentToMainstate_String(scriptargs[i])) {
            pushfailure = 1;
            break;
        }
        i++;
    }
    if (pushfailure) {
        printerror("Error: Couldn't push all script arguments into script state");
        main_Quit(1);
        return 1;
    }

    // open and run provided script file and pass the command line arguments:
    if (!luastate_DoInitialFile(script, scriptargcount, &error)) {
        if (error == NULL) {
            error = outofmem;
        }
        printerror("Error: An error occured when running \"%s\": %s", script, error);
        if (error != outofmem) {
            free(error);
        }
        fatalscripterror();
        main_Quit(1);
        return 1;
    }

#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: Calling blitwiz.on_init...");
#endif

    // call init
    if (!luastate_CallFunctionInMainstate("blitwiz.on_init", 0, 1, 1, &error, NULL)) {
        printerror("Error: An error occured when calling blitwiz.on_init: %s",error);
        if (error != outofmem) {
            free(error);
        }
        fatalscripterror();
        main_Quit(1);
        return 1;
    }

    // when graphics or audio is open, run the main loop
    int blitwizonstepworked = 0;
    int blitwizondrawworked = 0;
#if defined(ANDROID) || defined(__ANDROID__)
    printinfo("Blitwizard startup: Entering main loop...");
#endif

    // Initialise audio when it isn't
    main_InitAudio();

    // If we failed to initialise audio, we want to simulate it
#ifdef USE_AUDIO
    uint64_t simulateaudiotime = 0;
    if (simulateaudio) {
        simulateaudiotime = time_GetMilliseconds();
    }
#endif

    uint64_t logictimestamp = time_GetMilliseconds();
    uint64_t lastdrawingtime = 0;
    uint64_t physicstimestamp = time_GetMilliseconds();
    while (!wantquit) {
        blitwizonstepworked = 1;
        blitwizondrawworked = 0;
        uint64_t time = time_GetMilliseconds();

        // this is a hack for SDL bug http://bugzilla.libsdl.org/show_bug.cgi?id=1422

#ifdef USE_AUDIO
        // simulate audio
        if (simulateaudio) {
            while (simulateaudiotime < time_GetMilliseconds()) {
                audiomixer_GetBuffer(48 * 4 * 2);
                simulateaudiotime += 1; // 48 * 1000 times * 4 bytes * 2 channels per second = simulated 48kHz 32bit stereo audio
            }
        }
#endif // ifdef USE_AUDIO

        // slow sleep: check if we can safe some cpu by waiting longer
        unsigned int deltaspan = 16;
#ifndef USE_GRAPHICS
        int nodraw = 1;
#else
        int nodraw = 1;
        if (graphics_AreGraphicsRunning()) {nodraw = 0;}
#endif
        uint64_t delta = time_GetMilliseconds()-lastdrawingtime;
        if (nodraw) {
            // we can sleep as long as our timeste allows us to
            deltaspan = ((double)TIMESTEP)/2.1f;
        }

        // sleep/limit FPS as much as we can
        if (delta < deltaspan) {
            if (connections_NoConnectionsOpen() && !listeners_HaveActiveListeners()) {
                time_Sleep(deltaspan-delta);
                connections_SleepWait(0);
            }else{
                connections_SleepWait(deltaspan-delta);
            }
        }else{
            connections_SleepWait(0);
        }

        // Remember drawing time and process net events
        lastdrawingtime = time_GetMilliseconds();
        if (!luafuncs_ProcessNetEvents()) {
            // there was an error processing the events
            main_Quit(1);
        }

#ifdef USE_GRAPHICS
        // check and trigger all sort of input events
        graphics_CheckEvents(&quitevent, &mousebuttonevent, &mousemoveevent, &keyboardevent, &textevent, &putinbackground);
#endif

        // call the step function and advance physics
        int iterations = 0;
        while ((logictimestamp < time || physicstimestamp < time) && iterations < MAXLOGICITERATIONS) {
            if (logictimestamp < time && logictimestamp <= physicstimestamp) {
                int onstepdoesntexist = 0;
                if (!luastate_CallFunctionInMainstate("blitwiz.on_step", 0, 1, 1, &error, &onstepdoesntexist)) {
                    printerror("Error: An error occured when calling blitwiz.on_step: %s", error);
                    if (error) {free(error);}
                    fatalscripterror();
                    main_Quit(1);
                    blitwizonstepworked = 0;
                }else{
                    if (onstepdoesntexist) {
                        blitwizonstepworked = 0;
                    }
                }
                logictimestamp += TIMESTEP;
            }
#ifdef USE_PHYSICS2D
            if (physicstimestamp < time && physicstimestamp <= logictimestamp) {
                int psteps = ((float)TIMESTEP/(float)physics2d_GetStepSize(physics2ddefaultworld));
                if (psteps < 1) {psteps = 1;}
                while (psteps > 0) {
                    physics2d_Step(physics2ddefaultworld);
                    physicstimestamp += physics2d_GetStepSize(physics2ddefaultworld);
                    psteps--;
                }
            }
#else
            physicstimestamp = time + 2000;
#endif
            iterations++;
        }

        // check if we ran out of iterations:
        if (iterations >= MAXLOGICITERATIONS) {
            if (
#ifdef USE_PHYSICS2D
                    physicstimestamp < time ||
#endif
                 logictimestamp < time) {
                // we got a problem: we aren't finished,
                // but we hit the iteration limit
                if (physicstimestamp < time || logictimestamp < time) {
                    physicstimestamp = time_GetMilliseconds();
                    logictimestamp = time_GetMilliseconds();
                    printwarning("Warning: logic is too slow, maximum logic iterations have been reached (%d)", (int)MAXLOGICITERATIONS);
                }
            }else{
                // we don't need to iterate anymore -> everything is fine
            }
        }

#ifdef USE_GRAPHICS
        // check for image loading progress
        if (!appinbackground) {
            graphics_CheckTextureLoading(&imgloadedcallback);
        }
#endif

#ifdef USE_GRAPHICS
        if (graphics_AreGraphicsRunning()) {
#ifdef ANDROID
            if (!appinbackground) {
#endif
                // start drawing
                drawingallowed = 1;
                graphicsrender_StartFrame();

                // call the drawing function
                int ondrawdoesntexist = 0;
                if (!luastate_CallFunctionInMainstate("blitwiz.on_draw", 0, 1, 1, &error, &ondrawdoesntexist)) {
                printerror("Error: An error occured when calling blitwiz.on_draw: %s",error);
                    if (error) {free(error);}
                    fatalscripterror();
                    main_Quit(1);
                }else{
                    if (!ondrawdoesntexist) {blitwizondrawworked = 1;}
                }

                // complete the drawing
                drawingallowed = 0;
                graphicsrender_CompleteFrame();
#ifdef ANDROID
            }else{
                blitwizondrawworked = 1;
            }
#endif
        }
#endif

        // we might want to quit if there is nothing else to do
#ifdef USE_AUDIO
        if (!blitwizondrawworked && !blitwizonstepworked && connections_NoConnectionsOpen() && !listeners_HaveActiveListeners() && audiomixer_NoSoundsPlaying()) {
#else
        if (!blitwizondrawworked && !blitwizonstepworked && connections_NoConnectionsOpen() && !listeners_HaveActiveListeners()) {
#endif
            main_Quit(1);
        }

#ifdef USE_GRAPHICS
        // be very sleepy if in background
        if (appinbackground) {
            time_Sleep(20);
        }
#endif

        // do some garbage collection:
        gcframecount++;
        if (gcframecount > 100) {
            // do a gc step once in a while
            luastate_GCCollect();
        }
    }
    main_Quit(0);
    return 0;
}

