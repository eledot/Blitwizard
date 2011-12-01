
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

#include "audiosource.h"
#include "audiosourceresample.h"


static void audiosourcefadepanvol_Close(struct audiosource* source) {
	struct audiosourceogg_internaldata* idata = source->internaldata;
	
	//close the processed source
	if (idata->source) {
		idata->source->close(idata->source);
	}
	
	//free all structs
	if (source->internaldata) {
		free(source->internaldata);
	}
	free(source);
}

struct audiosource* audiosourceresample_Create(struct audiosource* source, int targetrate) {
	if (targetrate != 48000 && source->samplerate != targetrate) {return NULL;}
	struct audiosource* a = malloc(sizeof(*a));
	if (!a) {
		return NULL;
	}
	
	memset(a,0,sizeof(*a));
	a->internaldata = malloc(sizeof(struct audiosourcefadepanvol_internaldata));
	if (a->internaldata) {
		free(a);
		return NULL;
	}
	memset(a->internaldata, 0, sizeof(*(a->internaldata)));
	
	struct audiosourcefadepanvol_internaldata* idata = a->internaldata;
	idata->source = source;
	a->samplerate = source->samplerate;
	
	a->read = &audiosourcefadepanvol_Read;
	a->close = &audiosourcefadepanvol_Close;
	
	return NULL;
}


