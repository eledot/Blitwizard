
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

#include "luastate.h"
#include "file.h"
#include "graphics.h"

int wantquit;
int main(int argc, char** argv) {
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
			printf("Error: Unknown option: %s\n",argv[i]);
			return -1;
		}else{
			if (!scriptargfound) {
				scriptargfound = 1;
				script = argv[i];
			}
		}
		i++;
	}
	char outofmem[] = "Out of memory";
	char* error;
	char* filenamebuf = NULL;
	if (file_IsDirectory(script)) {
		filenamebuf = file_AddComponentToPath(script, "game.lua");
		script = filenamebuf;
	}
	if (option_changedir) {
		char* p = file_GetAbsoluteDirectoryPathFromFilePath(script);
		if (!p) {
			printf("Error: NULL returned for absolute directory\n");
			return -1;
		}
		char* newfilenamebuf = file_GetFileNameFromFilePath(script);
		if (!newfilenamebuf) {
			free(p);
			printf("Error: NULL returned for file name\n");
			return -1;
		}
		if (filenamebuf) {free(filenamebuf);}
		filenamebuf = newfilenamebuf;
		if (!file_Cwd(p)) {
			free(filenamebuf);
			printf("Error: Cannot cd to \"%s\"\n",p);
			free(p);
			return -1;
		}
		free(p);
		script = filenamebuf;
	}
	if (!luastate_DoInitialFile(script, &error)) {
		if (error == NULL) {
			error = outofmem;
		}
		printf("Error when running \"%s\": %s\n",script,error);
		if (filenamebuf) {free(filenamebuf);}
		return -1;
	}
	if (graphics_AreGraphicsRunning()) {
		while (1) {
		
		}
	}
	return 0;
}

