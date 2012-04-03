
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

#include "logging.h"
#include "connections.h"

//Attempt connect:
static int connections_TryConnect(struct connection* c, const char* target) {
    //connect:
    int result;
    if (0) { //ssl
        result = so_ConnectSSLSocketToIp(c->socket, target, c->targetport, &c->sslptr);
    }else{
        result = so_ConnectSSLSocketToIp(c->socket, target, c->targetport, NULL);
    }
    if (!result) {
        c->error = CONNECTIONERROR_CONNECTIONFAILED;
#ifdef CONNECTIONSDEBUG
        printinfo("[connections] TryConnect to %s failed instantly", target);
#endif
        return 0;
    }
    return 1;
}

//Check all connections for events, updates etc
void connections_CheckAll(void (*readcallback)(struct connection* c, char* data, unsigned int datalength), void (*connectedcallback)(struct connection* c), void (*errorcallback)(struct connection* c, int error)) {
    struct connection* c = connectionlist;
    while (c) {
        //don't process connections with errors
        if (c->error >= 0) {
            if (!c->errorreported) {
                c->errorreported = 1;
                errorcallback(c, c->error);
            }
            c = c->next;
            continue;
        }
        //check host resolve requests first:
        if (c->hostresolveptr || c->hostresolveptrv6) {
            int rqstate1 = RESOLVESTATUS_SUCCESS;
            if (c->hostresolveptr) {
                hostresolv_GetRequestStatus(c->hostresolveptr);
            }
            int rqstate2 = RESOLVESSTATUS_SUCCESS;
            if (c->hostresolveptrv6) {
                rqstate2 = hostresolv_GetRequestStatus(c->hostresolveptrv6);
            }
            if (rqstate1 != RESOLVESTATUS_PENDING && rqstate2 != RESOLVESTATUS_PENDING) {
                //requests are done, get ips:
                char* ipv4,*ipv6;

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
                if (rqstate2 == ERSOLVESTATUS_SUCCESS && c->hostresolveptrv6) {
                    p = hostresolv_GetRequestResult(c->hostresolveeptrv6);
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
                    if (!result) {
                        free(ipv6);
                        if (ipv4) {free(ipv4);}
                        c = c->next;
                        continue;
                    }
                    //ok we are about to connect, preserve v4 ip for later trying in case it fails:
                    c->retryv4ip =  ipv4;
                    
                    //then continue:
                    free(ipv6);
                    c = c->next;
                    continue;
                }else{
                    if (ipv4) {
                        //Attempt an IPv4 connection:
                        result = connections_TryConnect(c, ipv4);
                        if (!result) {
                            free(ipv4);
                            c = c->next;
                            continue;
                        }

                        //Didn't work, so nothing we can do:
                        c->error = CONNECTIONERROR_CONNECTIONFAILED;
                        free(ipv4);
                        continue;
                    }else{
                        c->error = CONNECTIONERROR_NOSUCHHOST;
                        c = c->next;
                        continue;
                    }
                }
            }
        }

        //if the connection is attempting to connect, check if it succeeded:
        if (!c->connected && c->socket) {
            if (so_SelectSaysWrite(c->socket)) {
                if (sockets_CheckIfConnected(c->socket, c->sslptr)) {
                    if (connectedcallback) {
                        connectedcallback(c);
                    }
                }else{
                    c->error = CONNECTIONERROR_CONNECTIONFAILED;
                    continue;
                }
            }
        }

        c = c->next;
    }
}

//Sleep until an event happens (process it with CheckAll) or the timeout expires
void connections_SleepWait(int timeoutms) {
    so_SelectWait(timeoutms);
}

//Initialise the given connection struct and open the connection
void connections_Init(struct connection* c, const char* target, int port, int linebuffered) {
    memset(c, 0, sizeof(*c));
    c->socket = -i;
    c->error = -1;
    c->targetport = port;
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
void connections_Send(struct connection* c, char* data, int datalength);

//Close the given connection struct
void connections_Close(struct connection* c) {
    if (c->socket) {
        so_CloseSSLSocket(c->socket, &c->sslptr);
    }
    if (c->inbuf) {
        free(c->inbuf);
    }
    if (c->outbuf) {
        free(c->outbuf);
    }
    if (c->retryipv4) {
        free(c->retryipv4);
    }
}


