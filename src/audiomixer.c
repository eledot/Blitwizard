
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
#include <stdio.h>

#include "audio.h"
#include "audiosource.h"

int mixerdisabled = 0;

struct soundchannel {
	struct audiosource* soundsource;
	
	struct audiosource* fadepanvolsource;
	struct audiosource* loopsource;
	
	int fadestart;
	int fadeend;
	float fadestartvalue;
	float fadeendvalue;
	float volume;
	float panning;
	int loop;
};

void audiomixer_Disable() {
	//this is called when we have no sound output at all -> disables sound processing
	mixerdisabled = 1;
}

char mixedaudiobuf[256];
int mixedaudiobuflen = 0;

static void audiomixer_FillMixAudioBuffer() { //SOUND THREAD
	
}

char* streambuf = NULL;
unsigned int streambuflen = 0;
void* audiomixer_GetBuffer(unsigned int len) { //SOUND THREAD
	if (streambuflen != len) {
		if (streambuf) {free(streambuf);}
		streambuf = malloc(len);
		streambuflen = len;
	}
	memset(streambuf, 0, len);
	
	return streambuf;
}

