
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
#include <stdio.h>
#ifdef WINDOWS
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif


#include "win32console.h"
#include "logging.h"

static int consoleopen = 0;

void win32console_Launch(void) {
#ifdef WINDOWS
    if (consoleopen) {
        return;
    }

    AllocConsole();

    HANDLE outhandle = GetStdHandle(STD_OUTPUT_HANDLE);
    int outfhandle = _open_osfhandle((long)outhandle, _O_TEXT);
    FILE* outfile = _fdopen(outfhandle, "w");
    setvbuf(outfile, NULL, _IONBF, 1);
    *stdout = *outfile;
    *stderr = *outfile;

#endif
    consoleopen = 1;

    // now print all the log that is still kept in memory:
    if (memorylogbuf) {
        printf("%s", memorylogbuf);
    }
}

void win32console_Close(void) {
#ifdef WINDOWS
    if (!consoleopen) {
        return;
    }

    fclose(stdout);
    FreeConsole();
#endif
    consoleopen = 0;
}

