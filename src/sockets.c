
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
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "sockets.h"

#ifdef USE_SOCKETS

#ifndef WINDOWS
#include <sys/types.h>
#include <sys/select.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#if (HAVE_MALLOC_H != 0)
#include <malloc.h>
#endif
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#endif

#ifdef WINDOWS
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <malloc.h>
#endif

#include "ipcheck.h"
#include "sockets.h"
#include "strings.h"

#ifdef USESSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#ifdef USESSL
SSL_CTX* ctx = NULL; //our used SSL context
struct sslinfo {
    SSL* sslhandle;
    int state_accepted;
    int managedbyselect; //1 yes, -1 no, 0: unknown
    int ssl_settosend; //was originally not on writelist, but now is due to SSL
    int ssl_requiresend; //is explicitely on writelist, but needs to stay there on removal due to SSL
    int ssl_writewantstoreceive; //the SSL_write() does want to receive! so SSL_read() shouldn't unset us because it just wants to write
};
#endif

//our set of select sets:
fd_set weircdselectset_readers;
fd_set weircdselectset_writers;
fd_set* readset = NULL; //we use that one just for reading
fd_set* writeset = NULL; //we use that one just for writing
int fdnr = 0;

#ifdef USESSL
static char staticsslmemoryerror[] = "Error message allocation error";
static int sslerror_memoryerror = 0; //1 if we had issues to allocate the error message
static char* sslerror = NULL;

static void promptsslerror(const char* str) {
    if (sslerror_memoryerror == 0) {
        //only free if not static buffer
        if (sslerror) {free(sslerror);}
    }
    sslerror_memoryerror = 0;
    sslerror = malloc(strlen(str)+1); //allocate error buffer
    if (sslerror) {
        strcpy(sslerror,str);
    }else{
        //unable to allocate error message -> static buffer
        sslerror = staticsslmemoryerror;
        sslerror_memoryerror = 1;
    }
}
#endif

int so_sysinited = 0;
static int soinitfunc(const char* cert, const char* key) {
    if (so_sysinited == 1) {return 1;}
    FD_ZERO(&weircdselectset_readers);
    FD_ZERO(&weircdselectset_writers);
#ifdef WINDOWS
    //Launch WSA sockets on Windows
    WSADATA wsa;
    long r = WSAStartup(MAKEWORD(2,0),&wsa);
    if (r != 0) {
        return 0;
    }
#else
    //deactivate SIGPIPE on Unix
    struct sigaction signalst;
    if (sigaction(SIGPIPE, NULL, &signalst) == 0) {
        signalst.sa_handler = SIG_IGN;
        sigaction(SIGPIPE, &signalst, NULL);
    }else{
        return 0;
    }
#endif
    if (readset == NULL) {
        readset = malloc(sizeof(weircdselectset_readers));
        if (readset == NULL) {
            return 0;
        }
    }
    if (writeset == NULL) {
        writeset = malloc(sizeof(weircdselectset_writers));
        if (writeset == NULL) {
            return 0;
        }
    }
    FD_ZERO(writeset);FD_ZERO(readset);
#ifdef USESSL
    if (cert && key) {
        SSL_library_init();
        SSL_load_error_strings();
        
        if ((ctx = SSL_CTX_new(SSLv23_server_method())) == NULL ||
        SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) {
            promptsslerror("Failed to load private key or certificate");
            SSL_CTX_free(ctx);
            so_sysinited = 1;
            return 1;
        }else{
            if (!SSL_CTX_check_private_key(ctx)) { //private key and public key don't match
                promptsslerror("Private key and public certificate do not match");
                SSL_CTX_free(ctx);
                so_sysinited = 1;
                return 1;
            }
        }
#ifndef SSLNOCLEAROPTIONS
        SSL_CTX_clear_options(ctx, SSL_OP_ALL);
#endif
        SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2|SSL_OP_CIPHER_SERVER_PREFERENCE|SSL_OP_TLS_ROLLBACK_BUG);
        SSL_CTX_set_mode(ctx, SSL_MODE_ENABLE_PARTIAL_WRITE|SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
    }else{
        promptsslerror("SSL supported but not activated");
    }
#endif
    so_sysinited = 1;
    return 1;
}
int so_Startup() {
    return soinitfunc(NULL, NULL);
}
int so_StartupWithSSL(const char* pathtocertificate, const char* pathtokey) {
    return soinitfunc(pathtocertificate, pathtokey);
}
char staticnosslinitbuf[] = "Sockets not initialized yet";
char staticnosslsupportbuf[] = "Compiled without SSL support";
const char* so_SSLNotAvailable() {
#ifdef USESSL
    if (so_sysinited != 1) {
        return staticnosslinitbuf;
    }
    return sslerror;
#else
    return staticnosslsupportbuf;
#endif
}
void so_SetSocketNonblocking(int socket) {
    if (so_sysinited == 0) {return;}
#ifndef WINDOWS
    int save_fd;
    save_fd = fcntl(socket, F_GETFL);
    save_fd |= O_NONBLOCK;
    fcntl(socket, F_SETFL, save_fd);
#endif
#ifdef WINDOWS
    long l = 1;
    ioctlsocket(socket,FIONBIO,(unsigned long*)&l);
#endif
}
int so_ReverseResolveBlocking(const char* ip, char* hostbuf, int hostbuflen) {
#ifdef IPV6
    struct sockaddr_in6 addressstruct6;
#endif
    struct sockaddr_in addressstruct4;
    struct sockaddr* sa = NULL;
    char tempip[IPMAXLEN+1];
    strncpy(tempip,ip,IPMAXLEN+1);
    tempip[IPMAXLEN] = 0;
    if (tempip[0] == '[') {memmove(tempip, tempip+1, IPMAXLEN);}
    if (tempip[strlen(tempip)-1] == ']') {tempip[strlen(tempip)-1] = 0;}
    int salen;
    int iptype = IPTYPE_IPV4;
    unsigned int r = 0;
    while (r < strlen(ip)) {
        if (ip[r] == '.') {break;}//is ipv4
        if (ip[r] == ':') {iptype = IPTYPE_IPV6;break;}
        r++;
    }
    if (iptype == IPTYPE_IPV6) {
        #ifdef IPV6
            memset(&addressstruct6,0,sizeof(addressstruct6));
            addressstruct6.sin6_family = AF_INET6;
            #ifdef WINDOWS
            DWORD size = sizeof(addressstruct6.sin6_addr);
            if (WSAStringToAddress(ip, AF_INET6, NULL, &(addressstruct6.sin6_addr), &size) != 0) {
                return 0;
            }
            #else
            inet_pton(AF_INET6, ip, &(addressstruct6.sin6_addr));
            #endif
            sa = (struct sockaddr*)&addressstruct6;
            salen = sizeof(struct sockaddr_in6);
        #else
            return 0;
        #endif
    }else{
        memset(&addressstruct4,0,sizeof(addressstruct4));
        addressstruct4.sin_family = AF_INET;
        #ifdef WINDOWS
        addressstruct4.sin_addr.s_addr = inet_addr(ip);
        #else
        inet_pton(AF_INET, ip, &(addressstruct4.sin_addr.s_addr));
        #endif
        sa = (struct sockaddr*)&addressstruct4;
        salen = sizeof(struct sockaddr_in);
    }
    int returnvalue = getnameinfo(sa, salen, hostbuf, hostbuflen, NULL, 0, NI_DGRAM|NI_NAMEREQD);
    if (returnvalue != 0) {
        return 0;
    }
    hostbuf[hostbuflen-1] = 0; //make sure it's always nullterminated even if truncated
    if (strlen(hostbuf) <= 0) {return 0;}
    return 1;
}
int so_ResolveBlocking(const char* host, int iptype, char* ipbuf, int ipbuflen) {
    struct addrinfo hints;
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_INET;
    if (iptype == IPTYPE_IPV6) {
        hints.ai_family = AF_INET6;
    }
    struct addrinfo *result = NULL;
    if (getaddrinfo(host, NULL, &hints, &result) != 0) {return 0;}
    if (result && iptype == IPTYPE_IPV4) {
        struct sockaddr_in  *sockaddr_ipv4;
        sockaddr_ipv4 = (struct sockaddr_in *)result->ai_addr;
        strncpy(ipbuf, inet_ntoa(sockaddr_ipv4->sin_addr), ipbuflen);
        ipbuf[ipbuflen-1] = 0;
        freeaddrinfo(result);
        if (strlen(ipbuf) <= 0) {return 0;}
        return 1;
    }
    if (result && iptype == IPTYPE_IPV6) {
        #ifdef WINDOWS
        struct sockaddr* sockaddr;
        sockaddr = result->ai_addr;
        if (WSAAddressToString(sockaddr, (DWORD) result->ai_addrlen, NULL, ipbuf, &ipbuflen) != 0) {
            freeaddrinfo(result);
            return 0;
        }
        #else
        struct sockaddr_in6* address6 = (struct sockaddr_in6*)result->ai_addr;
        inet_ntop(AF_INET6, &(address6->sin6_addr), ipbuf, ipbuflen);
        ipbuf[ipbuflen-1] = 0;
        #endif
        freeaddrinfo(result);
        if (strlen(ipbuf) <= 0) {return 0;}
        return 1;
    }
    freeaddrinfo(result);
    return 0;
}

void so_ManageSocketWithSelect(int socket) {
    if (so_sysinited == 0) {return;}
    so_SetSocketNonblocking(socket);
    FD_SET(socket,&weircdselectset_readers);
    if (fdnr <= socket) {fdnr = socket+1;}
    return;
}
void so_SelectWantWrite(int socket, int enabled) {
    if (so_sysinited == 0) {return;}
    if (enabled == 1) {
        FD_SET(socket,&weircdselectset_writers);
    }else{
        FD_CLR(socket,&weircdselectset_writers);
    }
}
void so_SelectWantWriteSSL(int socket, int enabled, void** sslptr) {
    if (so_sysinited == 0) {return;}
#ifdef USESSL
    if (sslptr == NULL || *sslptr == NULL) {so_SelectWantWrite(socket,enabled);return;}
    struct sslinfo* i = *sslptr;
    if (enabled == 1) {
        FD_SET(socket,&weircdselectset_writers);
        if (i->ssl_settosend == 1) { //mark SSL wants to keep this when we remove it
            i->ssl_settosend = 0;
            i->ssl_requiresend = 1;
        }
    }else{
        if (i->ssl_requiresend == 1) {
            i->ssl_requiresend = 0;
            i->ssl_settosend = 1;
        }
        if (i->ssl_settosend == 1) {return;} //we really do want to keep it
        FD_CLR(socket,&weircdselectset_writers);
    }
#else
    so_SelectWantWrite(socket,enabled);
#endif
}
int so_CreateSocket(int addToSelect, int iptype) {
    if (so_sysinited == 0) {return -1;}
    int newsock = -1;
    if (iptype == IPTYPE_IPV6) {
#ifdef IPV6
        newsock = socket(AF_INET6, SOCK_STREAM, 0);
#endif  
    }
    if (iptype == IPTYPE_IPV4) {
        newsock = socket(AF_INET, SOCK_STREAM, 0);
    }
    if (newsock < 0) {return -1;}
    so_SetSocketNonblocking(newsock);
    if (addToSelect == 1) {so_ManageSocketWithSelect(newsock);}
    return newsock;
}
int so_MakeSocketListen(int socket, int port, int iptype, const char* bindip) {
    if (so_sysinited == 0) {return 0;}
    // ( 1 ) --- prepare address struct
#ifdef IPV6
    struct sockaddr_in6 addressstruct6;
#endif
    struct sockaddr_in addressstruct4;
    if (iptype == IPTYPE_IPV6) {
#ifdef IPV6
        memset(&addressstruct6, 0, sizeof(addressstruct6));
        addressstruct6.sin6_family = AF_INET6;
#else
        return  0;
#endif
    }else{
        memset(&addressstruct4, 0, sizeof(addressstruct4));
        addressstruct4.sin_family = AF_INET;
    }
    // ( 2 ) --- get the ip specified in "bindip" into the address struct
    if (strcasecmp(bindip,"any") != 0) {
        //1- now bind depending on operating system and ip family (ipv4 or ipv6)
        /* LINUX/UNIX */
#ifndef WINDOWS
        if (iptype == IPTYPE_IPV6) {
        #ifdef IPV6
            inet_pton(AF_INET6, bindip, &(addressstruct6.sin6_addr));
        #else
            return 0;
        #endif
        }else{
            inet_pton(AF_INET, bindip, &(addressstruct4.sin_addr.s_addr));
        }
#endif
        /* WINDOWS */
#ifdef WINDOWS
        if (iptype == IPTYPE_IPV6) {
        #ifdef IPV6
            DWORD size = sizeof(addressstruct6.sin6_addr);
            if (WSAStringToAddress(bindip, AF_INET6, NULL, &(addressstruct6.sin6_addr), &size) != 0) {
                return 0;
            }
        #else
            return 0;
        #endif
        }else{
            addressstruct4.sin_addr.s_addr = inet_addr(bindip);
        }
#endif

        //2- check if binding worked
        
        if (iptype == IPTYPE_IPV6) {
#ifdef IPV6
            if (IN6_IS_ADDR_UNSPECIFIED(&addressstruct6.sin6_addr) && strcasecmp(bindip, "::0") != 0) {
                return 0;
            }
#else
            return 0;
#endif
        }else{
            if (addressstruct4.sin_addr.s_addr == INADDR_NONE) {
                return 0;
            }
        }
    }else{
        if (iptype == IPTYPE_IPV6) {
#ifdef IPV6
            addressstruct6.sin6_addr = in6addr_any;
#else
            return 0;
#endif
        }else{
            addressstruct4.sin_addr.s_addr = INADDR_ANY;
        }
    }
    // ( 3 ) --- set port
    if (iptype == IPTYPE_IPV6) {
#ifdef IPV6
        addressstruct6.sin6_port = htons(port);
#else
        return 0;
#endif
    }else{
        addressstruct4.sin_port = htons(port);
    }
    // allow reusing the address on unix systems
#ifndef WINDOWS
    long lclyes = 1;
    setsockopt(socket, SOL_SOCKET,SO_REUSEADDR, &lclyes, sizeof(int));
#endif
    // ( 4 ) --- the actual binding process & listening
    if (iptype == IPTYPE_IPV6) {
#ifdef IPV6
        if (bind(socket,(struct sockaddr *) &addressstruct6,sizeof(addressstruct6)) < 0) {
            return 0;
        }
#else
        return 0;
#endif
    }else{
        if (bind(socket,(struct sockaddr *) &addressstruct4,sizeof(addressstruct4)) < 0) {
            return 0;
        }
    }
    listen(socket, port);
    return 1;
}
int so_AddressToStruct(const char* addr, int iptype, void* structptr) {
#ifdef IPV6
    struct sockaddr_in6* addressstruct6 = structptr;
#endif
    struct sockaddr_in* addressstruct4 = structptr;
    if (iptype == IPTYPE_IPV6) {
        #ifdef IPV6
        memset(addressstruct6, 0, sizeof(*addressstruct6));
        addressstruct6->sin6_family = AF_INET6;
        #else
        return 0;
        #endif
    }else{
        memset(addressstruct4, 0, sizeof(*addressstruct4));
        addressstruct4->sin_family = AF_INET;
    }
    if (strcasecmp(addr,"any") != 0) {
        // 1 - extract ip to struct depending on OS and ipv4/ipv6
        /* LINUX/UNIX */
#ifndef WINDOWS
        if (iptype == IPTYPE_IPV6) {
        #ifdef IPV6
            if (inet_pton(AF_INET6, addr, &(addressstruct6->sin6_addr)) != 1) {
                return 0;
            }
        #else
            return 0;
        #endif
        }else{
            if (inet_pton(AF_INET, addr, &(addressstruct4->sin_addr.s_addr)) != 1) {
                return 0;
            }
        }
#endif
        /* WINDOWS */
#ifdef WINDOWS
        if (iptype == IPTYPE_IPV6) {
        #ifdef IPV6
            DWORD size = sizeof(addressstruct6->sin6_addr);
            if (WSAStringToAddress(addr, AF_INET6, NULL, &(addressstruct6->sin6_addr), &size) != 0) {
                return 0;
            }
        #else
            return 0;
        #endif
        }else{
            addressstruct4->sin_addr.s_addr = inet_addr(addr);
        }
#endif

        // 2 - check if ip extraction worked
        
        if (iptype == IPTYPE_IPV6) {
#ifdef IPV6
            if (IN6_IS_ADDR_UNSPECIFIED(&addressstruct6->sin6_addr)) {
                return 0;
            }
#else
            return 0;
#endif
        }else{
            if (addressstruct4->sin_addr.s_addr == INADDR_ANY || addressstruct4->sin_addr.s_addr == INADDR_NONE) {
                return 0;
            }
        }
    }else{
        if (iptype == IPTYPE_IPV6) {
#ifdef IPV6
            addressstruct6->sin6_addr = in6addr_any;
#else
            return 0;
#endif
        }else{
            addressstruct4->sin_addr.s_addr = INADDR_ANY;
        }
    }
    return 1;
}
int so_ConnectSocketToIP(int socket, const char* ip, unsigned int port) {
    return so_ConnectSSLSocketToIP(socket, ip, port, NULL);
}
int so_ConnectSSLSocketToIP(int socket, const char* ip, unsigned int port, void** sslptr) {
    if (so_sysinited == 0) {return 0;}

#ifndef USESSL
    if (sslptr) {return 0;}
#else
    if (sslptr && sslerror) {return 0;}
#endif

    // ( 1 ) --- prepare address struct
#ifdef IPV6
    struct sockaddr_in6 addressstruct6;
#endif
    struct sockaddr_in addressstruct4;
    
    int iptype = IPTYPE_IPV4;
    if (isipv6ip(ip)) {iptype = IPTYPE_IPV6;}
    
    //clear struct
    if (iptype == IPTYPE_IPV6) {
#ifdef IPV6
        memset(&addressstruct6, 0, sizeof(addressstruct6));
#else
        return 0;
#endif
    }else{
        memset(&addressstruct4, 0, sizeof(addressstruct4));
    }
    
    // ( 2 ) --- fill in ip of struct
    if (iptype == IPTYPE_IPV6) {
#ifdef IPV6
        if (!so_AddressToStruct(ip, iptype, &addressstruct6)) {return 0;}
#endif
    }else{
        if (!so_AddressToStruct(ip, iptype, &addressstruct4)) {return 0;}
    }
    
    // ( 3 ) --- set port of struct
    if (iptype == IPTYPE_IPV6) {
#ifdef IPV6
        addressstruct6.sin6_port = htons(port);
#endif
    }else{
        addressstruct4.sin_port = htons(port);
    }

    // ( 3b ) -- initialize SSL
#ifdef USESSL
    if (sslptr != NULL && !sslerror && !*sslptr) {
        //initialize ssl thing
        *sslptr = malloc(sizeof(struct sslinfo));
        if (!(*sslptr)) {return 0;}
        memset(*sslptr,0,sizeof(struct sslinfo));
        ((struct sslinfo*)*sslptr)->sslhandle = SSL_new(ctx);
        if (!((struct sslinfo*)*sslptr)->sslhandle) {
            free(*sslptr);
            *sslptr = NULL;
            return 0;
        }
        SSL_set_fd(((struct sslinfo*)*sslptr)->sslhandle, socket);
    }
#endif
    
    // ( 4 ) --- connect!
    int r;
    if (iptype == IPTYPE_IPV6) {
        errno = 0;
        #ifdef IPV6
        r = connect(socket, (struct sockaddr*)&addressstruct6, sizeof(addressstruct6));
        #endif
    }else{
        errno = 0;
        r = connect(socket, (struct sockaddr*)&addressstruct4, sizeof(addressstruct4));
    }
    if (r == 0) {return 1;}
    
    //check for async error code
    #ifdef WINDOWS
    int err = WSAGetLastError();
    if (err == WSAEWOULDBLOCK || errno == WSAEINPROGRESS) {return 1;}
    #else
    if (errno == EWOULDBLOCK || errno == EINPROGRESS) {return 1;}
    #endif
    return 0;
}
int so_SelectWait(int maximumsleeptime) {
    if (so_sysinited == 0) {return 0;}
    struct timeval t;
    memset(&t,0,sizeof(struct timeval));
    t.tv_usec = maximumsleeptime*1000;
    memcpy(readset,&weircdselectset_readers,sizeof(weircdselectset_readers));
    memcpy(writeset,&weircdselectset_writers,sizeof(weircdselectset_writers));
    int r;
    //printf("DEBUG: select(): Call... (wantsend: %d)\n",wantsend);
    r = select(fdnr,readset,writeset,NULL,&t);
    if (r <= 0) {
        FD_ZERO(readset);FD_ZERO(writeset);
    }
    return r;
}
void so_CloseSocket(int socket) {
    if (so_sysinited == 0) {return;}
    FD_CLR(socket, &weircdselectset_readers);
    FD_CLR(socket, &weircdselectset_writers);
    if (readset != NULL) {FD_CLR(socket, readset);}
    if (writeset != NULL) {FD_CLR(socket, writeset);}
    close(socket);
}
int so_AcceptConnection_internal(int socket, int iptype, char* writeip, int* writesocket, void** sslptr) {
    if (so_sysinited == 0) {return 0;}
    socklen_t addrlen;

#ifndef USESSL
    if (sslptr) {return 0;}
#else
    if (sslptr && sslerror) {return 0;}
#endif

#ifdef IPV6
    struct sockaddr_in6 address6;
#endif
    struct sockaddr_in address4;
    if (iptype == IPTYPE_IPV6) {
        #ifdef IPV6
        addrlen = sizeof(address6);
        #else
        return 0;
        #endif
    }else{
        addrlen = sizeof(address4);
    }
    int new_socket;
    if (!FD_ISSET(socket,readset)) {return 0;}
    if (iptype == IPTYPE_IPV6) {
        #ifdef IPV6
        new_socket = accept(socket,(struct sockaddr *) &address6,&addrlen);
        #else
        return 0;
        #endif
    }else{
        new_socket = accept(socket,(struct sockaddr *) &address4,&addrlen);
#ifdef USESSL
        if (sslptr != NULL && !sslerror) {
            //initialize ssl thing
            *sslptr = malloc(sizeof(struct sslinfo));
            if (!(*sslptr)) {close(new_socket);return 0;}
            memset(*sslptr,0,sizeof(struct sslinfo));
            ((struct sslinfo*)*sslptr)->sslhandle = SSL_new(ctx);
            if (!((struct sslinfo*)*sslptr)->sslhandle) {
                free(*sslptr); *sslptr = NULL;
                close(new_socket);
                return 0;
            }
            SSL_set_fd(((struct sslinfo*)*sslptr)->sslhandle, new_socket);
        }
#endif
    }
    if (new_socket > 0) {
        so_ManageSocketWithSelect(new_socket);
        //write the ip address into the "writeip" buf
        if (writeip != NULL) {
            int cnt = IPMAXLEN+1;
            if (iptype == IPTYPE_IPV4) {
                cnt = IPV4LEN+1;
            }
#ifdef WINDOWS // WINDOZE
            if (iptype == IPTYPE_IPV6) {
                #ifdef IPV6
                getnameinfo((struct sockaddr *)&address6, sizeof(struct
                sockaddr_in6), writeip, cnt, NULL, 0, NI_NUMERICHOST);
                writeip[IPMAXLEN] = 0;
                #else
                #ifdef USESSL
                if (sslptr != NULL && !sslerror) {
                    //free SSL data again
                    SSL_free(((struct sslinfo*)*sslptr)->sslhandle);
                    free(*sslptr); *sslptr = NULL;
                    close(new_socket);
                }
                #endif
                return 0;
                #endif
            }else{
                getnameinfo((struct sockaddr *)&address4, sizeof(struct
                sockaddr_in), writeip, cnt, NULL, 0, NI_NUMERICHOST);
                writeip[IPV4LEN] = 0;
            }
#endif

#ifndef WINDOWS // LINUX/POSIX
            if (iptype == IPTYPE_IPV6) {
                #ifdef IPV6
                inet_ntop(AF_INET6, &address6.sin6_addr, writeip, cnt);
                writeip[IPMAXLEN] = 0;
                #else
                #ifdef USESSL
                if (sslptr != NULL && !sslerror) {
                    //free SSL stuff
                    SSL_free(((struct sslinfo*)*sslptr)->sslhandle);
                    free(*sslptr); *sslptr = NULL;
                    close(new_socket);
                }
                #endif
                return 0;
                #endif
            }else{
                inet_ntop(AF_INET, &address4.sin_addr, writeip, cnt);
                writeip[IPV4LEN] = 0;
            }
#endif
            
        }
        if (writesocket != NULL) {
            *writesocket = new_socket;
        }
        return 1;
    }
    return 0;
}
int so_AcceptConnection(int socket, int iptype, char* writeip, int* writesocket) {
    return so_AcceptConnection_internal(socket, iptype, writeip, writesocket, NULL);
}
int so_AcceptSSLConnection(int socket, int iptype, char* writeip, int* writesocket, void** sslptr) {
#ifdef USESSL
    if (sslerror || so_sysinited == 0) {
        return 0;
    }
    if (sslptr == NULL) {
        return so_AcceptConnection_internal(socket, iptype, writeip, writesocket, NULL);
    }
    return so_AcceptConnection_internal(socket, iptype, writeip, writesocket, sslptr);
#else
    return 0;
#endif
}
int so_GetIPLen() {
    return IPMAXLEN;
}
int so_SelectSaysWrite(int sock, void** sslptr) {
    if (so_sysinited == 0) {return 0;}
#ifdef USESSL
    if (sslptr && *sslptr) {
        struct sslinfo* i = *sslptr;
        if (i->ssl_writewantstoreceive) {
            //we had an SSL_write() which wants to read.
            //if _read_ is ready, claim we can actually write now (so SSL_write() gets repeated).
            if (FD_ISSET(sock, readset)) {
                return 1;
            }else{
                return 0;
            }
            return 0;
        }
        if (i->ssl_settosend || i->ssl_requiresend) {
            //we had an SSL_read() which wants to write.
            //claim we cannot write so it gets repeated, even if we could.
            return 0;
        }
    }
#endif
    if (!FD_ISSET(sock, writeset)) {
        return 0;
    }
    return 1;
}
int so_SelectSaysRead(int sock, void** sslptr) {
    if (so_sysinited == 0) {return 0;}
#ifdef USESSL
    if (sslptr && *sslptr) {
        struct sslinfo* i = *sslptr;
        if (i->ssl_writewantstoreceive) {
            //we had an SSL_write() which wants to read.
            //claim we cannot read so it gets repeated, even if we could.
            return 0;
        }
        if (i->ssl_settosend || i->ssl_requiresend) {
            //we had an SSL_read() which wants to write.
            //if _write_ is ready, claim we can read (so SSL_read() gets repeated).
            if (FD_ISSET(sock, writeset)) {
                return 1;
            }else{
                return 0;
            }
        }
    }
#endif
    if (!FD_ISSET(sock, readset)) {
        return 0;
    }
    return 1;
}
int dealwitherror(int retval) {
    if (retval > 0) {
        return retval;
    }else{
#ifdef WINDOWS
        int err = WSAGetLastError();
        if ((err != WSAEWOULDBLOCK && err != WSAEINTR && err != WSAENOBUFS) || retval == 0) {
#endif
#ifndef WINDOWS
        if ((errno != EWOULDBLOCK && errno != EINTR && errno != ENOBUFS) || retval == 0) {
#endif
            return 0;//EOF/fatal error which leads to stream stopping
        }else{
            return -1;//just a temporary error
        }
    }
}
#ifdef USESSL
int dealwithsslerror(int socket, void* sslptr, int arewereading, int retval) {
    struct sslinfo* i = sslptr;
    //unmark from writing first (if not set explicitely through user)
    if (i->managedbyselect == 1) {
        if (i->ssl_settosend == 1) {
            i->ssl_settosend = 0;
            FD_CLR(socket,&weircdselectset_writers);
        }
        i->ssl_requiresend = 0;
        i->ssl_writewantstoreceive = 0;
        FD_SET(socket,&weircdselectset_readers);
    }
    if (retval > 0) {
        ERR_clear_error();
        return retval;
    }else{
        int detailederror = SSL_get_error(i->sslhandle, retval);
        ERR_clear_error();
        if (detailederror == SSL_ERROR_ZERO_RETURN || detailederror == SSL_ERROR_SSL) {
            //ERR_print_errors_fp(stdout);
            //printf("SSL error or SSL zero return\n");
            return 0;
        }
        if (detailederror == SSL_ERROR_WANT_READ || detailederror == SSL_ERROR_WANT_WRITE) {
            if (detailederror == SSL_ERROR_WANT_WRITE && arewereading == 1) {
                //for SSL_read()'s write needs, we want to internally hack into the write fdset
                //why?
                //the user sets the write fdset for sockets for which they wants to WRITE, not
                //read data from (which SSL_read() does). therefore we do this hidden without
                //disrupting what the user sets from the outside when SSL_read() requires us
                //to read.
                if (i->managedbyselect == 1) {
                    if (i->ssl_settosend == 0 && i->ssl_requiresend == 0) {
                        if (FD_ISSET(socket, &weircdselectset_readers)) {
                            //it is already set. mark that we want to keep it
                            i->ssl_requiresend = 1;
                        }else{
                            //set it
                            i->ssl_settosend = 1;
                            FD_SET(socket,&weircdselectset_writers);
                        }
                    }
                    if (i->ssl_writewantstoreceive == 0) {
                        FD_CLR(socket,&weircdselectset_readers);
                    }
                }
            }else{
                if (detailederror == SSL_ERROR_WANT_READ && arewereading != 1) {
                    //for SSL_write()'s read needs, we want to make sure we are in the readset
                    if (i->managedbyselect == 1) {
                        i->ssl_writewantstoreceive = 1;
                        FD_SET(socket,&weircdselectset_readers);
                    }
                }
            }
            return -1;
        }
        if (detailederror == SSL_ERROR_SYSCALL) {
            return dealwitherror(-1);
        }
        return 0; //if in doubt, assume fatal error
    }
}
#endif
#ifdef USESSL
int so_DoAcceptThings(int socket, void* sslptr) {
    struct sslinfo* i = sslptr;
    if (i->state_accepted == 0) { //accept not done yet
        //unmark us from writing at first (if not explicitely user-set)
        if (i->managedbyselect == 1) {
            if (i->ssl_settosend == 1) {
                i->ssl_settosend = 0;
                FD_CLR(socket, &weircdselectset_writers);
            }
            i->ssl_requiresend = 0;
        }
        
        if (SSL_accept(i->sslhandle) < 0) {
            int r = SSL_get_error(i->sslhandle, -1);
            ERR_clear_error();
            if (r == SSL_ERROR_WANT_READ || r == SSL_ERROR_WANT_WRITE) {
                if (r == SSL_ERROR_WANT_WRITE) {
                    //mark us for writing
                    if (i->managedbyselect == 1) {
                        if (i->ssl_settosend == 0 && i->ssl_requiresend == 0) {
                            if (FD_ISSET(socket, &weircdselectset_readers)) {
                                //it is already set. mark that we want to keep it
                                i->ssl_requiresend = 1;
                            }else{
                                //set it
                                i->ssl_settosend = 1;
                                FD_SET(socket,&weircdselectset_writers);
                            }
                        }
                    }
                }
                return -1; //temporary problem
            }
            //printf("accept fail\n");
            //ERR_print_errors_fp(stdout);
            return 0; //constant problem
        }
        ERR_clear_error();
        i->state_accepted = 1;
    }
    return 1;
}
#endif
int so_SendData_internal(int socket, const char* buf, int len, int usessl, void* sslptr) {
    if (so_sysinited == 0) {return -1;}
    if (usessl == 0) {
        errno = 0;
        int z = send(socket,buf,len,0);
        return dealwitherror(z);
    }else{
#ifdef USESSL
        struct sslinfo* i = sslptr;
        if (i->ssl_settosend == 1 || i->ssl_requiresend == 1) {
            //SSL_read() was unfinished. therefore, don't allow SSL_write() before that SSL_read() is complete
            return -1;
        }
        if (i->managedbyselect == 0) {
            if (!FD_ISSET(socket, &weircdselectset_readers)) {
                i->managedbyselect = -1;
            }else{
                i->managedbyselect = 1;
            }
        }
        int z = so_DoAcceptThings(socket, sslptr);
        if (z <= 0) {
            return z;
        }
        z = SSL_write(i->sslhandle, buf, len);
        return dealwithsslerror(socket, sslptr, 0, z);
#else
        return -1;
#endif
    }
}

int so_CheckIfConnected(int socket, void** sslptr) {
#ifdef USESSL
    if (sslptr && *sslptr) {
        //XXX: is this sufficient for connection checks?
        int z = so_DoAcceptThings(socket, *sslptr);
        if (z <= 0) {
            return 0;
        }
        return 1;
    }
#endif
    int val;
    unsigned int len = sizeof(val);
    int result = getsockopt(socket, SOL_SOCKET, SO_ERROR, &val, &len);
    if (result < 0) { //failed to obtain option
        return 0;
    }
    if (val == 0) { //connect success
        return 1;
    }
    return 0;
}

int so_ReceiveData_internal(int socket, char* buf, int len, int usessl, void* sslptr) {
    if (so_sysinited == 0) {return -1;}
    if (usessl == 0) {
        int z = recv(socket,buf,len,0);
        return dealwitherror(z);
    }else{
#ifdef USESSL
        struct sslinfo* i = sslptr;
        if (i->ssl_writewantstoreceive) {
            //SSL_write() was unfinished. therefore, don't allow SSL_read() before that SSL_write() is complete
            return -1;
        }
        if (i->managedbyselect == 0) {
            if (!FD_ISSET(socket, &weircdselectset_readers)) {
                i->managedbyselect = -1;
            }else{
                i->managedbyselect = 1;
            }
        }
        int z = so_DoAcceptThings(socket, sslptr);
        if (z <= 0) {
            return z;
        }
        z = SSL_read(i->sslhandle, buf, len);
        return dealwithsslerror(socket, sslptr, 1, z);
#else
        return -1;
#endif
    }
}
int so_SendData(int socket, const char* buf, int len) {
    return so_SendData_internal(socket, buf, len, 0, NULL);
}
int so_ReceiveData(int socket, char* buf, int len) {
    return so_ReceiveData_internal(socket, buf, len, 0, NULL);
}
int so_SendSSLData(int socket, const char* buf, int len, void** sslptr) {
    if (sslptr == NULL || *sslptr == NULL) {
        return so_SendData_internal(socket, buf, len, 0, NULL);
    }
    return so_SendData_internal(socket, buf, len, 1, *sslptr);
}
int so_ReceiveSSLData(int socket, char* buf, int len, void** sslptr) {
    if (sslptr == NULL || *sslptr == NULL) {
        return so_ReceiveData_internal(socket, buf, len, 0, NULL);
    }
    return so_ReceiveData_internal(socket, buf, len, 1, *sslptr);
}
void so_CloseSSLSocket(int socket, void** sslptr) {
    so_CloseSocket(socket);
    if (sslptr) {
        if (*sslptr) {
#ifdef USESSL
            struct sslinfo* i = *sslptr;
            SSL_free(i->sslhandle);
#endif
            free(*sslptr);
            *sslptr = NULL;
        }
    }

}

#endif // ifdef USE_SOCKETS


