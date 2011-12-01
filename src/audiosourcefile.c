
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

#include <stdio.h>
#include <stdlib.h>

#include "audiosource.h"
#include "audiosourcefile.h"

struct audiosourcefile_internaldata {
	FILE* file;
	int eof = 0;
};

static int audiosourcefile_Read(struct audiosource* source, unsigned int bytes, char* buffer) {
	struct audiosourcefile_internaldata* idata = source->internaldata;
	
	if (idata->file == NULL) {
		if (idata->eof) {
			return -1;
		}
		idata->file = fopen(path,"rb");
		if (!idata->file) {
			idata->file = NULL;
			return -1;
		}
	}
	
	size_t bytesread = fread(buffer, bytes, 1, idata->file);
	if (bytesread >= 0) {
		return bytesread;
	}else{
		fclose(idata->file);
		idata->file = NULL;
		idata->eof = 1;
		return 0;
	}
}

static void audiosourcefile_Close(struct audiosource* source) {
	//close file we might have opened
	FILE* r = ((struct audiosourceogg_internaldata*)source->internaldata)->file;
	if (r) {
		free(r);
	}
	//free all structs
	if (source->internaldata) {
		free(source->internaldata);
	}
	free(source);
}

struct audiosource* audiosourcefile_Create(const char* path) {
	struct audiosource* a = malloc(sizeof(*a));
	if (!a) {
		return NULL;
	}
	
	memset(a,0,sizeof(*a));
	a->internaldata = malloc(sizeof(struct audiosourcefile_internaldata));
	if (a->internaldata) {
		free(a);
		return NULL;
	}
	memset(a->internaldata, 0, sizeof(*(a->internaldata)));
	
	a->read = &audiosourcefile_Read;
	a->close = &audiosourcefile_Close;
	
	return a;
}

