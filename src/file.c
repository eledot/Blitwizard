
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

#ifdef WIN
#include <windows.h>
#else
#include <limits.h>
#endif
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "file.h"

int file_Cwd(const char* path) {
#ifdef WIN
	if (SetCurrentDirectory(path) == 0) {
		return 0;
	}
#else
	if (chdir(basepath) != 0) {
		return 0;
	}
#endif
	return 1;
}

char* file_GetCwd() {
	char cwdbuf[2048] = "";
#ifdef WIN
	if (GetCurrentDirectory(sizeof(cwdbuf), cwdbuf) <= 0) {
		return 0;
	}
#else
	if (!getcwd(cwdbuf, sizeof(cwdbuf))) {
		return 0;
	}
#endif
	return strdup(cwdbuf);
}

int file_IsDirectory(const char* path) {
#ifndef WIN
	struct stat info;
	int r = stat(path,&info);
	if (r < 0) {return 0;}
	if (S_ISDIR(info.st_mode) != 0) {return 1;}
	return 0;
#endif
#ifdef WIN
	HANDLE hf = FindFirstFile(path, &findFileData);
	if (hf == INVALID_HANDLE_VALUE) {FindClose(hf);return 0;}
	FindClose(hf);
	if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
		return 1;
	}
	return 0;
#endif
}

char* file_GetAbsolutePathFromRelativePath(const char* relativepath) {
#ifdef WIN

#else
	return realpath(relativepath, NULL);
#endif
}

static int file_LatestSlash(const char* path) {
#ifdef WIN
	char slash = '\\';
#else
	char slash = '/';
#endif
	int i = strlen(path)-1;
	while (path[i] != slash && i > 0) {
		i--;
	}
	if (i <= 0) {i = -1;}
	return i;
}

char* file_GetDirectoryPathFromFilePath(const char* path) {
	if (file_IsDirectory(path)) {
		return strdup(path);
	}else{
		char* pathcopy = strdup(path);
		if (!pathcopy) {return NULL;}
		int i = file_LatestSlash(path);
		if (i < 0) {
			free(pathcopy);
			return strdup("");
		}else{
			pathcopy[i+1] = 0;
			return pathcopy;
		}
	}
}

int file_IsPathRelative(const char* path) {
#ifdef WIN
	if (PathIsRelative(path) == TRUE) {return 1;}
	return 0;
#else
	if (path[0] == '/') {return 0;}
	return 1;
#endif
}

char* file_GetAbsoluteDirectoryPathFromFilePath(const char* path) {
	char* p = file_GetDirectoryPathFromFilePath(path);
	if (!p) {return NULL;}
	
	if (!file_IsPathRelative(p)) {
		return p;
	}
	
	char* p2 = file_GetAbsolutePathFromRelativePath(p);
	if (!p2) {return NULL;}
	return p2;
}

char* file_GetFileNameFromFilePath(const char* path) {
	int i = file_LatestSlash(path);
	if (i < 0) {
		return strdup(path);
	}else{
		char* filename = malloc(strlen(path)-i);
		if (!filename) {return NULL;}
		memcpy(filename, path + i + 1, strlen(path)-i);
		return filename;
	}
}

int file_ContentToBuffer(const char* path, char** buf, size_t* buflen) {
	FILE* r = fopen(path, "rb");
	if (!r) {
		return 0;
	}
	//obtain file size
	fseek(r, 0L, SEEK_END);
	unsigned int size = ftell(r);
	fseek(r, 0L, SEEK_SET);
	//allocate buf
	char* fbuf = malloc(size+1);
	if (!fbuf) {
		fclose(r);
		return 0;
	}
	//read data
	int i = fread(fbuf, 1, size, r);
	fclose(r);
	//check data
	if (i != size) {
		free(fbuf);
		return 0;
	}
	*buflen = size;
	*buf = fbuf;
	return 1;
}

