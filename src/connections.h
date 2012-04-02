

#define CONNECTIONINBUFSIZE (1024*10)
#define CONNECTIONOUTBUFSIZE (1024*50)

struct connection {
    int socket;

    char* inbuf;
    int inbufsize;
    int inbufbytes;

    char* outbuf;
    int outbufsize;
    int outbufbytes;

    int iptype;
    int connected;
    int linebuffered;
    int error;
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


