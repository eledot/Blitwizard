
#include "connections.h"

//Check all connections for events, updates etc
void connections_CheckAll(void (*readcallback)(struct connection* c, char* data, unsigned int datalength), void (*connectedcallback)(struct connection* c), void (*errorcallback)(struct connection* c, int error)) {
    
}

//Sleep until an event happens (process it with CheckAll) or the timeout expires
void connections_SleepWait(int timeoutms);

//Initialise the given connection struct and open the connection
void connections_Init(struct connection* c, const char* target, int port, int linebuffered) {
    memset(c, 0, sizeof(*c));
    c->socket = -1;
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
}

//Send on a connection. Do not use if ->connected is not 1 yet!
void connections_Send(struct connection* c, char* data, int datalength);

//Close the given connection struct
void connections_Close(struct connection* c);

