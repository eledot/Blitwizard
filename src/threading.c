
/* blitwizard 2d engine - source code file

  Copyright (C) 2012 Jonas Thiem

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
#include <string.h>
#include "SDL.h"

struct mutex {
	SDL_mutex *m;
};

#include "threading.h"

mutex* mutex_Create() {
	mutex* m = malloc(sizeof(mutex));
	if (!m) {return NULL;}
	memset(m, 0, sizeof(*m));
	m->m = SDL_CreateMutex();
	return m;
} 

void mutex_Destroy(mutex* m) {
	SDL_DestroyMutex(m->m);
	free(m);
}

void mutex_Lock(mutex* m) {
	SDL_LockMutex(m->m);
}

void mutex_Release(mutex* m) {
	SLD_UnlockMutex(m->m);
}

