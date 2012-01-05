
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
#include <dlfcn.h>
#endif
#include <string.h>
#include <stdlib.h>

#include "library.h"

char fileext[10] = "";

const char* library_GetFileExtension() {
	if (strlen(fileext) > 0) {
		return fileext;
	}

	#ifdef WIN
	strcpy(fileext, ".dll");
	#else
	#if defined(__APPLE__) && defined(__MACH__)
	strcpy(fileext, ".dylib");
	#else
	strcpy(fileext, ".so");
	#endif
	#endif

	return fileext;
}

void* library_Load(const char* libname) {
	//Check if name ends with library extension
	const char* ext = library_GetFileExtension();
	unsigned int i = strlen(libname)-strlen(ext);
	unsigned int istart = i;
	int matches = 1;
	while (i < strlen(libname)) {
		if (libname[i] != ext[i-istart]) {
			matches = 0;
			break;
		}
		i++;
	}

	//Append extension if we don't have it
	char* freename = NULL;
	if (!matches) {
		//Compose new name
		freename = malloc(strlen(libname) + strlen(ext) + 1);
		if (!freename) {
			return NULL;
		}
		memcpy(freename, libname, strlen(libname));
		memcpy(freename + strlen(libname), ext, strlen(ext));
		freename[strlen(libname) + strlen(ext)] = 0;

		//Set it as name
		libname = (const char*)freename;
	}

	//Load library
	#ifdef WIN
	void* ptr = (void*)LoadLibrary(libname);
	#else
	void* ptr = dlopen(libname, RTLD_LAZY|RTLD_LOCAL);
	#endif

	//Clean up and return pointer
	if (freename) {
		free(freename);
	}
	return ptr;
}

void* library_GetSymbol(void* ptr, const char* name) {
	#ifdef WIN
	return (void*)GetProcAddress((HMODULE)ptr, name);
	#else
	return dlsym(ptr, name);
	#endif
}

void library_Close(void* ptr) {
	#ifdef WIN
	FreeLibrary((HMODULE)ptr);
	#else
	dlclose(ptr);
	#endif
}

