
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
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include "logging.h"
#include "os.h"
#ifdef WINDOWS
#include <windows.h>
#ifdef USE_SDL_GRAPHICS
#include "SDL.h"
#include "graphicstexture.h"
#include "graphics.h"
#endif
#endif

#define MEMORYLOGBUFCHUNK (1024 * 10)
char* memorylogbuf = NULL;
int memorylogbufsize = 0;
extern int suppressfurthererrors;

void memorylog(const char* str) {
    int newlen = strlen(str);
    int currlen = 0;
    if (memorylogbuf) {
        currlen = strlen(memorylogbuf);
    }

    // if the memory buffer is full, enlarge:
    if (currlen + newlen >= memorylogbufsize) {
        // check on new size
        int newsize = memorylogbufsize;
        while (currlen + newlen >= newsize) {
            newsize += MEMORYLOGBUFCHUNK;
        }
        char* p = realloc(memorylogbuf, newsize);
        // out of memory.. nothing we could sensefully do
        if (!p) {
            return;
        }
        // resizing complete:
        memorylogbuf = p;
        memorylogbufsize = newsize;
    }
    // append new log data:
    memcpy(memorylogbuf + currlen, str, newlen);
    memorylogbuf[currlen+newlen] = 0;
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
    memorylog(printline);
    memorylog("\n");
#endif
#ifdef WINDOWS
    // we want graphical error messages for windows
    if (!suppressfurthererrors || 1 == 1) {
        // minimize drawing window if fullscreen
#ifdef USE_GRAPHICS
        if (graphics_AreGraphicsRunning() && graphics_IsFullscreen()) {
            graphics_MinimizeWindow();
        }
#endif
        // show error msg
        char printerror[4096];
        snprintf(printerror, sizeof(printerror)-1,
        "The application cannot continue due to a fatal error:\n\n%s",
        printline);
#ifdef USE_SDL_GRAPHICS
        MessageBox(graphics_GetWindowHWND(), printerror, "Fatal error", MB_OK|MB_ICONERROR);
#else
        MessageBox(NULL, printerror, "Fatal error", MB_OK|MB_ICONERROR);
#endif
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
    memorylog(printline);
    memorylog("\n");
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
    memorylog(printline);
    memorylog("\n");
#endif
}



