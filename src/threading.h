
/* blitwizard game engine - source code file

  Copyright (C) 2012-2013 Jonas Thiem

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

#ifndef BLITWIZARD_THREADING_H_
#define BLITWIZARD_THREADING_H_

typedef struct mutex mutex;
typedef struct threadinfo threadinfo;
typedef struct semaphore semaphore;

// create, destroy a mutex:
mutex* mutex_Create(void);
void mutex_Destroy(mutex* m);

// lock and release a mutex:
void mutex_Lock(mutex* m);
void mutex_Release(mutex* m);

// create, destroy a semaphore:
semaphore* semaphore_Create(int value);  // init with given value
void semaphore_Destroy(semaphore* s);

// wait for a semaphore (decrementing it), and post (increment it again)
void semaphore_Wait(semaphore* s);
void semaphore_Post(semaphore* s);

// create threadinfo:
threadinfo* thread_CreateInfo(void);

// spawn a new thread:
void thread_Spawn(threadinfo* tinfo, void (*func)(void* userdata),
void* userdata);

// free threadinfo (you can safely do this when the thread is still running):
void thread_FreeInfo(threadinfo* tinfo);

#endif  // BLITWIZARD_THREADING_H_

