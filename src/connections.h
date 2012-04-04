
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

#ifndef _BLITWIZARD_CONNECTIONS_H

#define _BLITWIZARD_CONECTIONS_H

#define CONNECTIONINBUFSIZE (1024*10)
#define CONNECTIONOUTBUFSIZE (1024*50)

#define CONNECTIONSDEBUG

struct connection {
    int socket;
    void* sslptr;
    int iptype;

    char* inbuf;
    int inbufsize;
    int inbufbytes;

    char* outbuf;
    int outbufsize;
    int outbufbytes;

    char* retryv4ip;
    int connected;
    int linebuffered;
    int throwawaynextline;
    int error;

    struct connection* next;
};

extern struct connection* connectionlist;

#define CONNECTIONERROR_INITIALISATIONFAILED 0
#define CONNECTIONERROR_NOSUCHHOST 1
#define CONNECTIONERROR_CONNECTIONFAILED 2
#define CONNECTIONERROR_CONNECTIONCLOSED 3

//Check all connections for events, updates etc
void connections_CheckAll(void (*readcallback)(struct connection* c, char* data, unsigned int datalength), void (*connectedcallback)(struct connection* c), void (*errorcallback)(struct connection* c, int error));

//Sleep until an event happens (process it with CheckAll) or the timeout expires
void connections_SleepWait(int timeoutms);

//Initialise the given connection struct and open the connection
void connections_Init(struct connection* c, const char* target, int port, int linebuffered);

//Send on a connection. Do not use if ->connected is not 1 yet!
void connections_Send(struct connection* c, char* data, int datalength);

//Close the given connection struct
void connections_Close(struct connection* c);

#endif // _BLITWIZARD_CONNECTIONS_H

