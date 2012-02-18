
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
#include <stdint.h>
#include <stdarg.h>
#ifdef WIN
#include <windows.h>
#define _WINDOWS_
#endif
#if defined(ANDROID) || defined(__ANDROID__)
#include <android/log.h>
#endif

int wantquit = 0;
int suppressfurthererrors = 0;
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

#define TIMESTEP 16

struct physicsworld* physicsdefaultworld = NULL;
void* main_DefaultPhysicsPtr() {
	return physicsdefaultworld;
}

void main_Quit(int returncode) {
	if (sdlinitialised) {
		//audio_Quit(); //FIXME: workaround for http://bugzilla.libsdl.org/show_bug.cgi?id=1396
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
#if defined(ANDROID) || defined(__ANDROID__)
	__android_log_print(ANDROID_LOG_ERROR, "blitwizard", "%s", printline);
#else
	fprintf(stderr,"%s\n",printline);
	fflush(stderr);
#endif
#ifdef WIN
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
	if (!audio_Init(&audiomixer_GetBuffer, 0, p, 0, &error)) {
		if (error) {free(error);}
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
    }
	if (p) {
		free(p);
	}
}


static void quitevent() {
	char* error;
	if (!luastate_CallFunctionInMainstate("blitwiz.on_close", 0, 1, 1, &error)) {
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
    if (!luastate_CallFunctionInMainstate(funcname, 3, 1, 1, &error)) {
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
	if (!luastate_CallFunctionInMainstate("blitwiz.on_mousemove", 2, 1, 1, &error)) {
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
	if (!luastate_CallFunctionInMainstate(funcname, 1, 1, 1, &error)) {
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
	if (!luastate_CallFunctionInMainstate("blitwiz.on_text", 1, 1, 1, &error)) {
		printerror("Error when calling blitwiz.on_text: %s", error);
		if (error) {free(error);}
		fatalscripterror();
		return;
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
	if (!luastate_CallFunctionInMainstate("blitwiz.on_image", 2, 1, 1, &error)) {
		printerror("Error when calling blitwiz.on_image: %s", error);
		if (error) {free(error);}
		fatalscripterror();
		return;
	}
}

#if (defined(__ANDROID__) || defined(ANDROID))
int SDL_main(int argc, char** argv) {
#else
int main(int argc, char** argv) {
#endif

#if defined(ANDROID) || defined(__ANDROID__)
	printinfo("Blitwizard %s starting", VERSION);
#endif

	//evaluate command line arguments:
	const char* script = "game.lua";
	int i = 1;
	int scriptargfound = 0;
	int option_changedir = 0;
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
	if (file_DoesFileExist("templates/init.lua")) {
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
	if (!luastate_CallFunctionInMainstate("blitwiz.on_init", 0, 1, 1, &error)) {
		printerror("Error: An error occured when calling blitwiz.on_init: %s",error);
		if (error != outofmem) {	
			free(error);
		}
		fatalscripterror();
		main_Quit(1);
	}
	
	//when graphics or audio is open, run the main loop
	if (graphics_AreGraphicsRunning() || audioinitialised) {

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
			uint64_t time = time_GetMilliSeconds();
			
			//simulate audio
			if (simulateaudio) {
				while (simulateaudiotime < time_GetMilliSeconds()) {
					audiomixer_GetBuffer(48 * 2 * 2);
					simulateaudiotime += 1; // 48 * 1000 times * 2 byte * 2 channels per second = simulated 48kHz 16bit stereo audio
				}
			}

			//limit to roughly 60 FPS
            uint64_t delta = time_GetMilliSeconds()-lastdrawingtime;
			if (delta < 15) {
				time_Sleep(16-delta);
			}

			//first, call the step function and advance physics
			while (logictimestamp < time || physicstimestamp < time) {
				if (logictimestamp < time && logictimestamp < physicstimestamp) {
					if (!luastate_CallFunctionInMainstate("blitwiz.on_step", 0, 1, 1, &error)) {
						printerror("Error: An error occured when calling blitwiz.on_step: %s", error);
						if (error) {free(error);}
						fatalscripterror();
						main_Quit(1);
					}
					logictimestamp += TIMESTEP;
				}
				if (physicstimestamp < time && physicstimestamp < logictimestamp) {
					physics_Step(physicsdefaultworld);
					physicstimestamp += physics_GetStepSize(physicsdefaultworld);
				}
			}

			//check for image loading progress
			graphics_CheckTextureLoading(&imgloaded);
			
			//check and trigger all sort of input events
			graphics_CheckEvents(&quitevent, &mousebuttonevent, &mousemoveevent, &keyboardevent, &textevent);
			
			//start drawing
			drawingallowed = 1;
			graphics_StartFrame();
			
			//call the drawing function
			if (!luastate_CallFunctionInMainstate("blitwiz.on_draw", 0, 1, 1, &error)) {
				printerror("Error: An error occured when calling blitwiz.on_draw: %s",error);
				if (error) {free(error);}
				fatalscripterror();
				main_Quit(1);
			}
			
			//complete the drawing
			drawingallowed = 0;
			graphics_CompleteFrame();
		}
	}
	main_Quit(0);
	return 0;
}

