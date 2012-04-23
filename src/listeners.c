
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

#include "sockets.h"
#include "listeners.h"

struct listener {
    void* userdata;
    int socket;
    int port;
    int ssl;
    struct listener* next;
};

struct listener* listeners = NULL;

static struct listener* listeners_GetByPort(int port, struct listener** prev) {
    struct listener* p = NULL;
    struct listener* l = listeners;
    while (l) {
        if (l->port == port) {
            if (prev) {
                *prev = p;
            }
            return l;
        }
        p = l;
        l = l->next;
    }
    return NULL;
}

int listeners_Create(int port, int ssl, void* userdata) {
    if (listeners_GetByPort(port, NULL)) {return 0;}
    struct listener* l = malloc(sizeof(*l));
    if (!l) {return 0;}
    memset(l, 0, sizeof(*l));
    l->socket = so_CreateSocket(1, IPTYPE_IPV6);
    l->ssl = ssl;
    l->userdata = userdata;
    if (l->socket < 0) {
        free(l);
        return 0;
    }
    if (!so_MakeSocketListen(l->socket, port, IPTYPE_IPV6, "::1")) {
        so_CloseSocket(l->socket);
        free(l);
        return 0;
    }
    l->next = listeners;
    listeners = l;
    return 1;
}

int listeners_CheckForConnections(int (*newconnection)(int port, int socket, const char* ip, void* sslptr, void* userdata)) {
    struct listener* l = listeners;
    while (l) {
        if (so_SelectSaysRead(l->socket, NULL)) {
            //something interesting happened with this listener:
            char ipbuf[IPMAXLEN+1];
            int sock;
            void* sptr = NULL;
            int havenewconnection = 0;

            //accepting new connection:
            if (l->ssl) {
                if (so_AcceptConnection(l->socket, IPTYPE_IPV6, ipbuf, &sock)) {
                    havenewconnection = 1;
                }
            }else{
                if (so_AcceptSSLConnection(l->socket, IPTYPE_IPV6, ipbuf, &sock, &sptr)) {
                    havenewconnection = 1;
                }
            }

            //process new connection if we have one:
            if (havenewconnection) {
                if (!newconnection(l->port, sock, ipbuf, sptr, l->userdata)) {
                    return 0;
                }
            }
        }
        l = l->next;
    }
    return 1;
}

int listeners_CloseByPort(int port) {
    struct listener* prev;
    struct listener* l = listeners_GetByPort(port, &prev);
    if (!l) {return 0;}
    so_CloseSocket(l->socket);
    if (prev) {
        prev->next = l->next;
    }else{
        listeners = l->next;
    }
    free(l);
    return 1;
}

