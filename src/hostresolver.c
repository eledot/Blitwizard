
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

#include <stdio.h>
#include <stdlib.h>
#include "sockets.h"

#ifdef USE_SOCKETS

#include <string.h>
#include "hostresolver.h"

#ifdef WINDOWS
#include <windows.h>
#include <process.h>
#endif

#ifndef WINDOWS
#include <pthread.h>
#endif

#define REQUESTTYPE_REVERSE 1
#define REQUESTTYPE_LOOKUP 2
struct requestinfo {
    int type;
    int wantipv6; // only used for lookups
#ifdef WINDOWS
    // windows threads stuff
    HANDLE threadhandle;
#else
    // pthreads for unix
    pthread_t threadhandle;
    pthread_mutex_t threadeventmutex;
    int threadeventobject;
#endif
    char* requestvalue;
    char* result;
};

struct cancelledrequest {
    void* handle;
    struct cancelledrequest* next;
};

static struct cancelledrequest* cancellist = NULL;

#ifdef WINDOWS
unsigned __stdcall resolvethread(void* data) {
#else
void* resolvethread(void* data) {
#endif
    struct requestinfo* i = data;
    if (i->type == REQUESTTYPE_REVERSE) {
        i->result = malloc(400);
        if (!so_ReverseResolveBlocking(i->requestvalue, i->result, 400)) {
            free(i->result);i->result = NULL;
        }
    }
    if (i->type == REQUESTTYPE_LOOKUP) {
        i->result = malloc(IPMAXLEN+1);
        int iptype = IPTYPE_IPV4;
        if (i->wantipv6 == 1) {
            iptype = IPTYPE_IPV6;
        }
        if (!so_ResolveBlocking(i->requestvalue, iptype, i->result, IPMAXLEN+1)) {
            free(i->result);i->result = NULL;
        }
    }
#ifdef WINDOWS
    return 0;
#else
    pthread_mutex_lock(&i->threadeventmutex);
    i->threadeventobject = 1;
    pthread_mutex_unlock(&i->threadeventmutex);
    pthread_exit(NULL);
    return NULL;
#endif
}
void* hostresolv_internal_NewRequest(int type, const char* requestvalue, int wantipv6) {
    // do either a reverse lookup or normal lookup request
    struct requestinfo i;
    memset(&i,0,sizeof(i));
    i.requestvalue = malloc(strlen(requestvalue)+1);
    if (!i.requestvalue) {
        return NULL;
    }
    strcpy(i.requestvalue, requestvalue);
    i.type = type;
    void* entry = malloc(sizeof(i));
    if (!entry) {
        free(i.requestvalue);
        return NULL;
    }
    memcpy(entry, &i, sizeof(i));
    // now fire off our thread
    struct requestinfo* ri = entry;
    ri->wantipv6 = wantipv6;
#ifdef WINDOWS
    ri->threadhandle = (HANDLE)_beginthreadex(NULL, 0, resolvethread, ri, 0, NULL);
#else
    pthread_mutex_init(&ri->threadeventmutex, NULL);
    ri->threadeventobject = 0;
    pthread_create(&ri->threadhandle, NULL, resolvethread, ri);
    pthread_detach(ri->threadhandle);
#endif
    return entry;
}
void* hostresolv_ReverseLookupRequest(const char* ip) {
    return hostresolv_internal_NewRequest(REQUESTTYPE_REVERSE, ip, 0);
}
void* hostresolv_LookupRequest(const char* host, int ipv6) {
    return hostresolv_internal_NewRequest(REQUESTTYPE_LOOKUP, host, ipv6);
}
void hostresolv_CleanCancelled() {
    struct cancelledrequest* c = cancellist;
    struct cancelledrequest* prev = NULL;
    while (c) {
        int state = hostresolv_GetRequestStatus(c->handle);
        if (state != RESOLVESTATUS_PENDING) {
            hostresolv_FreeRequest(c->handle);
            c->handle = NULL;
            if (prev) {
                prev->next = c->next;
            }else{
                cancellist = c->next;
            }
            struct cancelledrequest* delete = c;
            c = c->next;
            free(delete);
            continue;
        }
        prev = c;
        c = c->next;
    }
}
int hostresolv_GetRequestStatus(void* handle) {
    if (!handle) {
        return RESOLVESTATUS_FAILURE;
    }
    struct requestinfo* i = handle;
#ifdef WINDOWS
    if (WaitForSingleObject(i->threadhandle, 0) == WAIT_OBJECT_0) {
#else
    pthread_mutex_lock(&i->threadeventmutex);
    int u = i->threadeventobject;
    pthread_mutex_unlock(&i->threadeventmutex);
    if (u == 1) {
#endif
        if (i->result != NULL) {
            return RESOLVESTATUS_SUCCESS;
        }else{
            return RESOLVESTATUS_FAILURE;
        }
    }else{
        return RESOLVESTATUS_PENDING;
    }
}
const char* hostresolv_GetRequestResult(void* handle) {
    hostresolv_CleanCancelled();
    if (!handle) {
        return NULL;
    }
    if (hostresolv_GetRequestStatus(handle) == RESOLVESTATUS_PENDING) {
        return NULL;
    }
    struct requestinfo* i = handle;
    return i->result;
}
void hostresolv_FreeRequest(void* handle) {
    if (!handle) {
        return;
    }
    struct requestinfo* i = handle;
    free(i->requestvalue);
    if (i->result) {
        free(i->result);
    }
#ifdef WINDOWS
    CloseHandle(i->threadhandle);
#else

#endif
    free(handle);
}
void hostresolv_CancelRequest(void* handle) {
    if (!handle) {
        return;
    }
    int state = hostresolv_GetRequestStatus(handle);
    if (state != RESOLVESTATUS_PENDING) {
        hostresolv_FreeRequest(handle);
        return;
    }
    struct cancelledrequest* c = malloc(sizeof(*c));
    c->handle = handle;
    if (cancellist) {
        c->next = cancellist;
    }else{
        c->next = NULL;
    }
    cancellist = c;
}

#endif // ifdef USE_SOCKETS
