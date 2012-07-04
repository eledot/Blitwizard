
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

#include "os.h"

#include <stdlib.h>
#include <string.h>
#ifdef HAVE_SDL
#include "SDL.h"
#else
#ifdef WINDOWS
#include <windows.h>
#else // ifdef HAVE_WINDOWS
#include <pthread.h>
#endif // ifdef HAVE_WINDOWS
#endif // ifdef HAVE_SDL

struct mutex {
#ifdef HAVE_SDL
    SDL_mutex *m;
#else
#ifdef WINDOWS
    HANDLE m;
#else
    pthread_mutex_t m;
#endif
#endif
};

#include "threading.h"

mutex* mutex_Create() {
    mutex* m = malloc(sizeof(mutex));
    if (!m) {return NULL;}
    memset(m, 0, sizeof(*m));
#ifdef HAVE_SDL
    m->m = SDL_CreateMutex();
#else
#ifdef WINDOWS
    m->m = CreateMutex(NULL, FALSE, NULL);
#else
    pthread_mutex_init(&m->m, NULL);
#endif
#endif
    return m;
}

void mutex_Destroy(mutex* m) {
#ifdef HAVE_SDL
    SDL_DestroyMutex(m->m);
#else
#ifdef WINDOWS
    CloseHandle(m->m);
#else
    pthread_mutex_destroy(&m->m);
#endif
#endif
    free(m);
}

void mutex_Lock(mutex* m) {
#ifdef HAVE_SDL
    SDL_LockMutex(m->m);
#else
#ifdef WINDOWS
    WaitForSingleObject(m->m, INFINITE);
#else
    pthread_mutex_lock(&m->m);
#endif
#endif
}

void mutex_Release(mutex* m) {
#ifdef HAVE_SDL
    SDL_UnlockMutex(m->m);
#else
#ifdef WINDOWS
    ReleaseMutex(m->m);
#else
    pthread_mutex_unlock(&m->m);
#endif
#endif
}

