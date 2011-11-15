
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

extern int drawingallowed; //stored in luafuncs.c
#include "luastate.h"
#include "file.h"
#include "graphics.h"
#include "timefuncs.h"

#define TIMESTEP 16

void fatalscripterror() {

}

void printerror(const char* fmt, ...) {
	char printline[2048];
	va_list a;
	va_start(a, fmt);
	vsnprintf(printline, sizeof(printline)-1, fmt, a);
	printline[sizeof(printline)-1] = 0;
	va_end(a);
	fprintf(stderr,"%s",printline);
	fflush(stderr);
}

static void quitevent() {
	char* error;
	if (!luastate_CallFunctionInMainstate("blitwiz.on_close", 0, 1, 1, &error)) {
		printerror("Error when calling blitwiz.on_close: %s\n",error);
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
                printerror("Error when pushing func args to %s\n",funcname);
                fatalscripterror();
                return;
        }
	if (!luastate_PushFunctionArgumentToMainstate_Double(x)) {
		printerror("Error when pushing func args to %s\n",funcname);
		fatalscripterror();
		return;
	}
	if (!luastate_PushFunctionArgumentToMainstate_Double(y)) {
		printerror("Error when pushing func args to %s\n",funcname);
                fatalscripterror();
                return;
        }
        if (!luastate_CallFunctionInMainstate(funcname, 3, 1, 1, &error)) {
                printerror("Error when calling %s: %s\n", funcname, error);
                if (error) {free(error);}
                fatalscripterror();
                return;
        }
}
static void mousemoveevent(int x, int y) {
	char* error;
	if (!luastate_PushFunctionArgumentToMainstate_Double(x)) {
                printerror("Error when pushing func args to blitwiz.on_mousemove\n");
                fatalscripterror();
                return;
        }
        if (!luastate_PushFunctionArgumentToMainstate_Double(y)) {
                printerror("Error when pushing func args to blitwiz.on_mousemove\n");
                fatalscripterror();
                return;
        }
        if (!luastate_CallFunctionInMainstate("blitwiz.on_mousemove", 2, 1, 1, &error)) {
                printerror("Error when calling blitwiz.on_mousemove: %s\n", error);
                if (error) {free(error);}
                fatalscripterror();
                return;
        }
}
static void keyboardevent(const char* button, int release) {
	char* error;
	char onkeyup[] = "blitwiz.on_keyup";
	const char* funcname = "blitwiz.on_keydown";
	if (release) {funcname = onkeyup;}
        if (!luastate_PushFunctionArgumentToMainstate_String(button)) {
                printerror("Error when pushing func args to %s\n", funcname);
                fatalscripterror();
                return;
        }
        if (!luastate_CallFunctionInMainstate(funcname, 1, 1, 1, &error)) {
                printerror("Error when calling %s: %s\n", funcname, error);
                if (error) {free(error);}
                fatalscripterror();
                return;
        }
}
static void textevent(const char* text) {
	char* error;
	if (!luastate_PushFunctionArgumentToMainstate_String(text)) {
                printerror("Error when pushing func args to blitwiz.on_text\n");
                fatalscripterror();
                return;
        }
        if (!luastate_CallFunctionInMainstate("blitwiz.on_text", 1, 1, 1, &error)) {
                printerror("Error when calling blitwiz.on_text: %s\n", error);
                if (error) {free(error);}
                fatalscripterror();
                return;
        }
}


static void imgloaded(int success, const char* texture) {
	char* error;
	if (!luastate_PushFunctionArgumentToMainstate_String(texture)) {
		printerror("Error when pushing func args to blitwiz.on_image\n");
		fatalscripterror();
		return;
	}
	if (!luastate_PushFunctionArgumentToMainstate_Bool(success)) {
        printerror("Error when pushing func args to blitwiz.on_image\n");
        fatalscripterror();
        return;
    }
	if (!luastate_CallFunctionInMainstate("blitwiz.on_image", 2, 1, 1, &error)) {
		printerror("Error when calling blitwiz.on_image: %s\n", error);
		if (error) {free(error);}
		fatalscripterror();
		return;
	}
}

int wantquit = 0;
int main(int argc, char** argv) {
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
			printerror("Error: Unknown option: %s\n",argv[i]);
			return -1;
		}else{
			if (!scriptargfound) {
				scriptargfound = 1;
				script = argv[i];
			}
		}
		i++;
	}

	//check the provided path:
	char outofmem[] = "Out of memory";
	char* error;
	char* filenamebuf = NULL;

	//check if a folder:
	if (file_IsDirectory(script)) {
		filenamebuf = file_AddComponentToPath(script, "game.lua");
		script = filenamebuf;
	}

	//check if we want to change directory to the provided path:
	if (option_changedir) {
		char* p = file_GetAbsoluteDirectoryPathFromFilePath(script);
		if (!p) {
			printerror("Error: NULL returned for absolute directory\n");
			return -1;
		}
		char* newfilenamebuf = file_GetFileNameFromFilePath(script);
		if (!newfilenamebuf) {
			free(p);
			printerror("Error: NULL returned for file name\n");
			return -1;
		}
		if (filenamebuf) {free(filenamebuf);}
		filenamebuf = newfilenamebuf;
		if (!file_Cwd(p)) {
			free(filenamebuf);
			printerror("Error: Cannot cd to \"%s\"\n",p);
			free(p);
			return -1;
		}
		free(p);
		script = filenamebuf;
	}

	//initialise graphics
	if (!graphics_Init(&error)) {
		printerror("Error when initialising graphics: %s\n",error);
		free(error);
		return -1;
	}
	
	//open and run provided file
	if (!luastate_DoInitialFile(script, &error)) {
		if (error == NULL) {
			error = outofmem;
		}
		printerror("Error when running \"%s\": %s\n",script,error);
		if (filenamebuf) {free(filenamebuf);}
		return -1;
	}

	//call init
	if (!luastate_CallFunctionInMainstate("blitwiz.on_init", 0, 1, 1, &error)) {
		printerror("Error when calling blitwiz.on_init: %s\n",error);
		return -1;
	}
	
	//when graphics are open, run the main loop
	if (graphics_AreGraphicsRunning()) {
		uint64_t logictimestamp = time_GetMilliSeconds();
		uint64_t lastdrawingtime = 0;
		while (!wantquit) {
			uint64_t time = time_GetMilliSeconds();

			//limit to roughly 60 FPS
                        uint64_t delta = time_GetMilliSeconds()-lastdrawingtime;
                        if (delta < 15) {
                                time_Sleep(16-delta);
                        }

			//first, call the step function
			while (logictimestamp < time) {
				if (!luastate_CallFunctionInMainstate("blitwiz.on_step", 0, 1, 1, &error)) {
					printerror("Error when calling blitwiz.on_step: %s\n", error);
					if (error) {free(error);}
				}
				logictimestamp += TIMESTEP;
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
				printerror("Error when calling blitwiz.on_draw: %s\n",error);
				if (error) {free(error);}
			}
			
			//complete the drawing
			drawingallowed = 0;
			graphics_CompleteFrame();
		}
	}
	return 0;
}

