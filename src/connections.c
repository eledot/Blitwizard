
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
#ifdef UNIX
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif
#ifdef WINDOWS
#include <winsock2.h>
#endif

#include "ipcheck.h"
#include "logging.h"
#include "sockets.h"
#include "connections.h"
#include "hostresolver.h"

struct connection* connectionlist;

//Attempt connect:
static int connections_TryConnect(struct connection* c, const char* target) {
    //connect:
    int result;
#ifdef CONNECTIONSDEBUG
    printinfo("[connections] TryConnect: %s:%d (%d)", target, c->targetport, c->socket);
#endif
    if (0) { //ssl
        result = so_ConnectSSLSocketToIP(c->socket, target, c->targetport, &c->sslptr);
    }else{
        result = so_ConnectSSLSocketToIP(c->socket, target, c->targetport, NULL);
    }
    if (!result) {
        c->error = CONNECTIONERROR_CONNECTIONFAILED;
#ifdef CONNECTIONSDEBUG
        printinfo("[connections] TryConnect to %s failed instantly", target);
#endif
        return 0;
    }else{
        //when the connection is complete, the socket will be writable:
        so_SelectWantWrite(c->socket, 1);
        //that is why we want to check for write state.
    }
    return 1;
}

//Set error and report it back immediately:
static int connections_E(struct connection* c, int (*errorcallback)(struct connection* c, int error), int error) {
    if (!c->errorreported) {
        c->error = error;
        if (errorcallback) {
            if (!errorcallback(c, error)) {return 0;}
        }
    }
    return 1;
}

//Check all connections for events, updates etc
struct connection* justreadingfromconnection = NULL;
int readconnectionclosed = 0;
int connections_CheckAll(int (*readcallback)(struct connection* c, char* data, unsigned int datalength), int (*connectedcallback)(struct connection* c), int (*errorcallback)(struct connection* c, int error)) {
    //initialise socket system (just in case it's not done yet):
    if (!so_Startup()) {
        return 1;
    }

    struct connection* c = connectionlist;
    while (c) {
        struct connection* cnext = c->next;
        //don't process connections with errors
        if (c->error >= 0) {
            if (!c->errorreported) {
                c->errorreported = 1;
                if (errorcallback) {
                    if (!errorcallback(c, c->error)) {return 0;}
                }
            }
            c = cnext;
            continue;
        }
        //check host resolve requests first:
        if (c->hostresolveptr || c->hostresolveptrv6) {
            int rqstate1 = RESOLVESTATUS_SUCCESS;
            if (c->hostresolveptr) {
                hostresolv_GetRequestStatus(c->hostresolveptr);
            }
            int rqstate2 = RESOLVESTATUS_SUCCESS;
            if (c->hostresolveptrv6) {
                rqstate2 = hostresolv_GetRequestStatus(c->hostresolveptrv6);
            }
            if (rqstate1 != RESOLVESTATUS_PENDING && rqstate2 != RESOLVESTATUS_PENDING) {
                //requests are done, get ips:
                char* ipv4 = NULL;
                char* ipv6 = NULL;

                //ipv4 ip:
                const char* p = NULL;
                if (rqstate1 == RESOLVESTATUS_SUCCESS && c->hostresolveptr) {
                    p = hostresolv_GetRequestResult(c->hostresolveptr);
                }
                if (p) {
                    ipv4 = strdup(p);
                }

                //ipv6 ip:
                p = NULL;
                if (rqstate2 == RESOLVESTATUS_SUCCESS && c->hostresolveptrv6) {
                    p = hostresolv_GetRequestResult(c->hostresolveptrv6);
                }
                if (p) {
                    ipv6 = strdup(p);
                }
                
                //free requests:
                if (c->hostresolveptr) {hostresolv_CancelRequest(c->hostresolveptr);}
                if (c->hostresolveptrv6) {hostresolv_CancelRequest(c->hostresolveptrv6);}
                c->hostresolveptr = NULL;
                c->hostresolveptrv6 = NULL;

#ifdef CONNECTIONSDEBUG
                printinfo("[connections] resolved host, results: v4: %s, v6: %s", ipv4, ipv6);
#endif
                
                //if we got a v6 ip, connect there first:
                int result;
                if (ipv6) {
                    //Prefer an IPv6 connection:
                    result = connections_TryConnect(c, ipv6);
                    free(ipv6);
                    if (!result) {
                        if (ipv4) {
                            //Try an IPv4 connection:
                            result = connections_TryConnect(c, ipv4);
                            free(ipv4);
                            if (!result) {
                                if (!connections_E(c, errorcallback, CONNECTIONERROR_CONNECTIONFAILED)) {
                                    return 0;
                                }
                            }
                        }
                        c = cnext;
                        continue;
                    }
                    //ok we are about to connect, preserve v4 ip for later trying in case it fails:
                    c->retryv4ip =  ipv4;
                    
                    //then continue:
                    c = cnext;
                    continue;
                }else{
                    if (ipv4) {
                        //Attempt an IPv4 connection:
                        result = connections_TryConnect(c, ipv4);
                        if (!result) {
                            free(ipv4);
                            c = cnext;
                            continue;
                        }

                        //Didn't work, so nothing we can do:
                        free(ipv4);
                        if (!connections_E(c, errorcallback, CONNECTIONERROR_CONNECTIONFAILED)) {
                            return 0;
                        }
                    }else{
                        if (!connections_E(c, errorcallback, CONNECTIONERROR_NOSUCHHOST)) {
                            return 0;
                        }
                    }
                    c = cnext;
                    continue;
                }
            }
        }

        //if the connection is attempting to connect, check if it succeeded:
        if (!c->connected && c->socket) {
            if (so_SelectSaysWrite(c->socket, &c->sslptr)) {
                if (c->outbufbytes <= 0) {
                    so_SelectWantWrite(c->socket, 0);
                }
                if (so_CheckIfConnected(c->socket, &c->sslptr)) {
#ifdef CONNECTIONSDEBUG
                    printinfo("[connections] now connected");
#endif
                    if (connectedcallback) {
                        if (!connectedcallback(c)) {return 0;}
                    }
                }else{
                    //we aren't connected!
                    if (c->retryv4ip) { //we tried ipv6, now try ipv4
#ifdef CONNECTIONSDEBUG
                        printinfo("[connections] retrying v4 after v6 fail...");
#endif
                        int result = connections_TryConnect(c, c->retryv4ip);
                        free(c->retryv4ip);
                        c->retryv4ip = NULL;
                        if (!result) {
                            if (!connections_E(c, errorcallback, CONNECTIONERROR_CONNECTIONFAILED)) {
                                return 0;
                            }
                        }
                        c = cnext;
                        continue;
                    }
#ifdef CONNECTIONSDEBUG
                    printinfo("[connections] connection couldn't be established");
#endif
                    if (!connections_E(c, errorcallback, CONNECTIONERROR_CONNECTIONFAILED)) {
                        return 0;
                    }
                    c = cnext;
                    continue;
                }
            }
        }

        //read things if we can:
        if (c->connected && so_SelectSaysRead(c->socket, &c->sslptr)) {
            if (c->inbufbytes >= c->inbufsize && c->linebuffered == 1) {
                //we will break this mega line into two:
                justreadingfromconnection = c;
                readconnectionclosed = 0;
                if (readcallback) {
                    if (!readcallback(c, c->inbuf, c->inbufbytes)) {return 0;}
                }
                if (readconnectionclosed) {
                    c = cnext;
                    continue;
                }
                c->inbufbytes = 0; //clear buffer
            }
            //read new bytes into the buffer:
            int r = so_ReceiveSSLData(c->socket, c->inbuf + c->inbufbytes, c->inbufsize - c->inbufbytes, &c->sslptr);
            if (r == 0)  {
                //connection closed. send out all data we still have:
                if (c->inbufbytes > 0) {
                    justreadingfromconnection = c;
                    readconnectionclosed = 0;
                    if (readcallback) {
                        if (!readcallback(c, c->inbuf, c->inbufbytes)) {return 0;}
                    }
                    if (readconnectionclosed) {
                        c = cnext;
                        continue;
                    }
                }
                //then error:
                if (!connections_E(c, errorcallback, CONNECTIONERROR_CONNECTIONCLOSED)) {
                    return 0;
                }
#ifdef CONNECTIONSDEBUG
                printinfo("[connections] receive returned end of stream");
#endif
                c = cnext;
                continue;
            }
            if (r > 0) { //we successfully received new bytes
                c->inbufbytes += r;
            }
        }

        //write things if we can:
        if (c->connected && c->outbufbytes > 0 && so_SelectSaysWrite(c->socket, &c->sslptr)) {
            int r = so_SendSSLData(c->socket, c->outbuf + c->outbufoffset, c->outbufbytes, &c->sslptr);
            if (r == 0) {
                //connection closed:
                if (!connections_E(c, errorcallback, CONNECTIONERROR_CONNECTIONCLOSED)) {
                    return 0;
                }
#ifdef CONNECTIONSDEBUG
                printinfo("[connections] send returned end of stream");
#endif
                c = cnext;
                continue;
            }
            //remove sent bytes from buffer:
            if (r > 0) {
                c->outbufbytes -= r;
                c->outbufoffset += r;
                if (c->outbufbytes <= 0) {
                    c->outbufoffset = 0;
                    so_SelectWantWrite(c->socket, 0);
                }else{
                    if (c->outbufoffset > CONNECTIONOUTBUFSIZE/2) {
                        //move buffer contents back to beginning
                        memmove(c->outbuf, c->outbuf + c->outbufoffset, c->outbufbytes);
                        c->outbufoffset = 0;
                    }
                }
            }
        }

        c = cnext;
    }
    return 1;
}

//Sleep until an event happens (process it with CheckAll) or the timeout expires
void connections_SleepWait(int timeoutms) {
    //initialise socket system (just in case it's not done yet):
    if (!so_Startup()) {
        return;
    }
    //use select to wait for events:
    so_SelectWait(timeoutms);
}

//Change linebuffered state:
void connections_SetLineBuffered(struct connection* c, int linebuffered) {
    c->linebuffered = linebuffered;
}

//Initialise the given connection struct and open the connection
void connections_Init(struct connection* c, const char* target, int port, int linebuffered, int lowdelay) {
    int oldluaref = c->luarefcount;
    memset(c, 0, sizeof(*c));
    c->luarefcount = oldluaref;
    c->socket = -1;
    c->error = -1;
    c->targetport = port;
    c->linebuffered = linebuffered;
    if (connectionlist) {
        c->next = connectionlist;
    }else{
        connectionlist = c;
    }
    //initialise socket system (just in case it's not done yet):
    if (!so_Startup()) {
        c->error = CONNECTIONERROR_INITIALISATIONFAILED;
        return;
    }
    //with an empty target, we simply won't do anything except instant-error:
    if (!target) {
        c->error = CONNECTIONERROR_NOSUCHHOST;
        return;
    }
    //allocate buffers:
    c->inbuf = malloc(CONNECTIONINBUFSIZE);
    if (!c->inbuf) {
        c->error = CONNECTIONERROR_INITIALISATIONFAILED;
        return;
    }
    c->outbuf = malloc(CONNECTIONOUTBUFSIZE);
    if (!c->outbuf) {
        c->error = CONNECTIONERROR_INITIALISATIONFAILED;
        return;
    }
    //get a socket
    if (isipv4ip(target)) { //only take ipv4 when it's obvious
        c->socket = so_CreateSocket(1, IPTYPE_IPV4);
        c->iptype = IPTYPE_IPV4;
    }else{ //otherwise, try ipv6 first
        c->socket = so_CreateSocket(1, IPTYPE_IPV6);
        c->iptype = IPTYPE_IPV6;
    }
    //set a low delay option if desired
    if (lowdelay) {
        int flag = 1;
        setsockopt(c->socket, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(int));
    }
    //without a socket, we cannot continue:
    if (c->socket < 0) {
        c->error = CONNECTIONERROR_INITIALISATIONFAILED;
        return;
    }
    //we probably need to resolve the host first:
    if (!isipv4ip(target) && !isipv6ip(target)) {
        c->hostresolveptr = hostresolv_LookupRequest(target, 0);
        c->hostresolveptrv6 = hostresolv_LookupRequest(target, 1);
        if (!c->hostresolveptrv6) { //we really want v6 resolution, so we work well with ipv6-only
            if (c->hostresolveptr) {hostresolv_CancelRequest(c->hostresolveptr);}
            c->hostresolveptr = NULL;
        }
#ifdef CONNECTIONSDEBUG
        printinfo("[connections] resolving target: %s",target);
#endif
        return;
    }
    //connect:
    int result = connections_TryConnect(c, target); 
    if (!result) {
        return;
    }
    //we want to know when ready for writing, since that means we're connected:
    so_SelectWantWrite(c->socket, 1);
#ifdef CONNECTIONSDEBUG
    printinfo("[connections] connecting to ip: %s", target);
#endif
}

//Send on a connection. Do not use if ->connected is not 1 yet!
void connections_Send(struct connection* c, char* data, int datalength) {
    int r = datalength;
    //sadly, we cannot send an infinite size of bytes:
    if (r > c->outbufsize - (c->outbufbytes + c->outbufoffset)) {
        r = c->outbufsize - (c->outbufbytes + c->outbufoffset);
    }
    if (r <= 0) {return;}
    //put bytes into send buffer:
    memcpy(c->outbuf + c->outbufoffset + c->outbufbytes, data, r);
    c->outbufbytes += r;
}

//Close the given connection struct
void connections_Close(struct connection* c) {
    if (justreadingfromconnection == c) {
        //if the trace goes back to a read callback,
        //the function triggering the callback wants to know
        //that this connection is now closed.
        readconnectionclosed = 1;
    }
    if (c->hostresolveptr) {
        hostresolv_CancelRequest(c->hostresolveptr);
    }
    if (c->hostresolveptrv6) {
        hostresolv_CancelRequest(c->hostresolveptrv6);
    }
    if (c->socket >= 0) {
        so_CloseSSLSocket(c->socket, &c->sslptr);
    }
    if (c->inbuf) {
        free(c->inbuf);
    }
    if (c->outbuf) {
        free(c->outbuf);
    }
    if (c->retryv4ip) {
        free(c->retryv4ip);
    }
    //Remove use from the list:
    struct connection* prevc = connectionlist;
    while (prevc && prevc->next != c) {
        prevc = prevc->next;
    }
    if (prevc) {
        prevc = c->next;
    }else{
        connectionlist = NULL;
    }
#ifdef CONNECTIONSDEBUG
    printinfo("[connections] connection closed and removed from the list.");
#endif
}

int connections_NoConnectionsOpen() {
    if (connectionlist) {return 0;}
    return 1;
}

