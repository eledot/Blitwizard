
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

#ifdef USE_SOCKETS

#define IPV6

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46 // needed for Microsoft Windows
#endif


#if (HAVE_OPENSSL_SSL_H==1)
#define USESSL
#endif

#define IPV4LEN 16

#ifndef IPMAXLEN
#ifdef IPV6
    #define IPV6LEN INET6_ADDRSTRLEN
    #define IPMAXLEN (INET6_ADDRSTRLEN+2)
#else
    #define IPMAXLEN 16
#endif
#endif


/* start sockets */
int so_Startup(); // initialize socket system

/* basic socket creation & handling */
int so_CreateSocket(int addToSelect, int iptype);
int so_MakeSocketListen(int socket, int port, int iptype, const char* bindip);
int so_ConnectSocketToIP(int socket, const char* ip, unsigned int port);
void so_CloseSocket(int socket);

/* the maximum length of a buffer that can hold all ips in string representation */
int so_GetIPLen();

/* === select() things === */

void so_ManageSocketWithSelect(int socket);
   // Mark a socket to be managed by select(). so_SelectWait() will then
   // wake up if there is something to read or write for this socket!

   // NOTE FOR SSL: please use this before ever receiving/sending data
   //  on the socket (immediately after creation) since the SSL code has
   //  to do some specific magic for managed sockets. If you use this after
   //  some reading/writing, the SSL code won't be aware of the change and
   //  fail to do this magic (which you don't want to run into).

int so_SelectWait(int maximumsleeptime);
   // If you marked any sockets with so_ManageSocketWithSelect(), this sleeps
   // until for one of all the marked sockets, there is something available
   // to be read or written.
   // Please subsequently check ALL sockets you marked with
   // so_SelectSaysWrite() and so_SelectSaysRead(), and if any of them is
   // told to have something to be read or written, read/write it all!
   // If you have nothing to write left, use so_SelectWantWrite(socket, 0).
   // Only after you did that, call this function again.

   // maximumsleeptime specifies an amount of time after which you want
   // to wake up if there is nothing to be read/write on any sockets.

   // Returns a value > 0 if any sockets have been marked (otherwise,
   // the timeout has triggered the wakeup).

int so_SelectSaysWrite(int sock, void** sslptr);
   // If this returns 1:
   // The socket is now, after so_SelectWait(), marked to be ready for writing.
   // Write something, or mark with so_SelectWantWrite(socket, 0).
   // If this returns 0:
   // Don't use so_SendData()/so_SendSSLData() since it won't work now anyway.
   // Note: For non-SSL connections, pass NULL for sslptr.

int so_SelectSaysRead(int socket, void** sslptr);
   // If this returns 1:
   // The socket is now, after so_SelectWait(), marked to be ready for reading.
   // Please use so_ReceiveData()/so_ReceiveSSLData() until they return a value
   // i <= 0.
   // If this returns 0:
   // Nothing to receive. Don't even bother trying, it will return failure.
   // Note: For non-SSL connections, pass NULL for sslptr.

void so_SelectWantWrite(int socket, int enabled);
   // Mark a socket which you marked with so_ManageSocketWithSelect() for writing.
   // so_SelectWait() will then wake up also if this socket is ready for writing
   // (otherwise it will only wake up if there is something to be read).
   // Specify 1 for enabled if you want to write something, otherwise 0.
   // If you specify 1 and then don't do a write when the socket is ready for it,
   // you'll run in a lot of ugly polling. Therefore set to 0 if all writing is
   // done.

/* === accept new connections, connect and send/recv data === */
int so_AcceptConnection(int socket, int iptype, char* writeip, int* writesocket); // 0: failure, 1: success
int so_ConnectSocketToIP(int socket, const char* ip, unsigned int port);
int so_CheckIfConnected(int socket, void** sslptr); // 0: not connected, 1: connected
int so_SendData(int sock,const char* buf, int len);
int so_ReceiveData(int socket, char* buf, int len); // >0: amonut of bytes read, -1: temp failure (retry later), 2: failure (connection dead)

/* === DNS === */
int so_ReverseResolveBlocking(const char* ip, char* hostbuf, int hostbuflen); // 0: resolve failure, 1: worked
int so_ResolveBlocking(const char* host, int iptype, char* ipbuf, int ipbuflen); // 0: resolve failure: 1 worked

/* === SSL initialization === */
int so_StartupWithSSL(const char* pathtocertificate, const char* pathtokey); // initialize with SSL
const char* so_SSLNotAvailable(); // if not NULL, then this contains an error message why there is no SSL

/* === SSL socket handling === */
// NOTE: all those functions perfectly work on normal sockets if you simply
// don't provide an sslptr (just set NULL for the sslptr parameters)

void so_SelectWantWriteSSL(int socket, int enabled, void** sslptr);
int so_AcceptSSLConnection(int socket, int iptype, char* writeip, int* writesocket, void** sslptr);
int so_ConnectSSLSocketToIP(int socket, const char* ip, unsigned int port, void** sslptr);
int so_SendSSLData(int sock, const char* buf, int len, void** sslptr);
int so_ReceiveSSLData(int socket, char* buf, int len, void** sslptr);
void so_CloseSSLSocket(int socket, void** sslptr);

#define IPTYPE_IPV4 1
#define IPTYPE_IPV6 2

#endif
