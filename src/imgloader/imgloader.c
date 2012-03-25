
/* blitwizard 2d engine - source code file

  Copyright (C) 2011 Jonas Thiem

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
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "pngloader.h"
#include "imgloader.h"

#if defined(_WIN32) || defined(_WIN64) || defined(WIN32) || defined(WIN64) || defined(__WIN32__) || defined(WINDOWS)
#define WIN
#include <windows.h>
#include <process.h>
#endif

#ifndef WIN
#include <pthread.h>
#endif

struct loaderthreadinfo {
    char* path;
    int (*readfunc)(void* buffer, size_t bytes, void* userdata);
    void* readfuncptr;
    void* memdata;
    unsigned int memdatasize;
    char* data;
    char* format;
    unsigned int datasize;
    int imagewidth,imageheight;
    int maxsizex,maxsizey;
    void(*callback)(void* handle, int imgwidth, int imgheight, const char* imgdata, unsigned int imgdatasize);
#ifdef WIN
    //windows threads stuff
    HANDLE threadhandle;
#else
    //pthreads for unix
    pthread_t threadhandle;
    pthread_mutex_t threadeventmutex;
    int threadeventobject;
#endif
};

#ifdef WIN
void loaderthreadfunction(void* data) {
#else
void* loaderthreadfunction(void* data) {
#endif
    struct loaderthreadinfo* i = data;
    // first, we probably need to load the image from a file first
    if (!i->memdata && i->path) {
        FILE* r = fopen(i->path, "rb");
        if (r) {
            char buf[512];
            int k = fread(buf, 1, sizeof(buf), r);
            while (k > 0) {
                void* newp = realloc(i->memdata, i->memdatasize + k);
                if (!newp) {
                    if (i->memdata) {free(i->memdata);}
                    i->memdata = NULL;
                    break;
                }
                i->memdata = newp;
                memcpy(i->memdata + i->memdatasize, buf, k);
                i->memdatasize += k;
                k = fread(buf, 1, sizeof(buf), r);
            }
            fclose(r);
        }
        free(i->path);
        i->path = NULL;
    }
    //load from a byte reading function
    if (i->readfunc) {
        void* p = NULL;
        size_t currentsize = 0;
        char buf[1024];

        //read byte chunks:
        int k = i->readfunc(buf, sizeof(buf), i->readfuncptr);
        while (k > 0) {
            currentsize += k;
            void* pnew = realloc(p, currentsize);
            if (!pnew) {
                while (k > 0) {k = i->readfunc(buf, sizeof(buf), i->readfuncptr);}
                k = -1;
                break;
            }
            p = pnew;
            //copy read bytes
            memcpy(p + (currentsize - k), buf, k);
            k = i->readfunc(buf, sizeof(buf), i->readfuncptr);
        }
        //handle error
        if (k < 0) {
            if (p) {
                free(p);
            }
        }else{
            i->memdata = p;
            i->memdatasize = currentsize;
        }
    }
    //now try to load the image!
    if (i->memdata) {
        if (i->memdatasize > 0) {
            if (!pngloader_LoadRGBA(i->memdata, i->memdatasize, &i->data, &i->datasize, &i->imagewidth, &i->imageheight, i->maxsizex, i->maxsizey)) {
                i->data = NULL;
                i->datasize = 0;
            }
        }
        free(i->memdata);
        i->memdata = NULL;
        
        //convert it if needed
        if (strcasecmp(i->format, "bgra") == 0) {
            img_ConvertRGBAtoBGRA(i->data, i->datasize);
        }
    }

    //enter callback if we got one
    if (i->callback) {
        i->callback(data, i->imagewidth, i->imageheight, i->data, i->datasize);
    }
    
#ifdef WIN
    return;
#else
    pthread_mutex_lock(&i->threadeventmutex);
    i->threadeventobject = 1;
    pthread_mutex_unlock(&i->threadeventmutex);
    pthread_exit(NULL);
    return NULL;
#endif
}

void startthread(struct loaderthreadinfo* i) {
#ifdef WIN
    i->threadhandle = (HANDLE)_beginthreadex(NULL, 0, loaderthreadfunction, i, 0, NULL);
#else
    pthread_mutex_init(&i->threadeventmutex, NULL);
    i->threadeventobject = 0;
    pthread_create(&i->threadhandle, NULL, loaderthreadfunction, i);
    pthread_detach(i->threadhandle);
#endif
}

void* img_LoadImageThreadedFromFile(const char* path, int maxwidth, int maxheight, const char* format, void(*callback)(void* handle, int imgwidth, int imgheight, const char* imgdata, unsigned int imgdatasize)) {
    struct loaderthreadinfo* t = malloc(sizeof(struct loaderthreadinfo));
    if (!t) {return NULL;}
    memset(t, 0, sizeof(*t));
    t->path = malloc(strlen(path)+1);
    if (!t->path) {
        free(t);
        return NULL;
    }
    t->format = malloc(strlen(format)+1);
    if (!t->format) {
        free(t->path);free(t);
        return NULL;
    }
    strcpy(t->path, path);
    strcpy(t->format, format);
    t->maxsizex = maxwidth; t->maxsizey = maxheight;
    startthread(t);
    return t;
}

void* img_LoadImageThreadedFromFunction(int (*readfunc)(void* buffer, size_t bytes, void* userdata), void* userdata, int maxwidth, int maxheight, const char* format, void(*callback)(int imgwidth, int imgheight, const char* imgdata, unsigned int imgdatasize)) {
    struct loaderthreadinfo* t = malloc(sizeof(struct loaderthreadinfo));
    if (!t) {return NULL;}
    memset(t, 0, sizeof(*t));
    t->readfunc = readfunc;
    t->readfuncptr = userdata;
    t->format = malloc(strlen(format)+1);
    if (!t->format) {
        free(t->memdata);free(t);
        return NULL;
    }
    strcpy(t->format, format);
    t->maxsizex = maxwidth;
    t->maxsizey = maxheight;
    startthread(t);
    return t;
}

void* img_LoadImageThreadedFromMemory(const void* memdata, unsigned int memdatasize, int maxwidth, int maxheight, const char* format, void(*callback)(int imgwidth, int imgheight, const char* imgdata, unsigned int imgdatasize)) {
    struct loaderthreadinfo* t = malloc(sizeof(struct loaderthreadinfo));
    if (!t) {return NULL;}
    memset(t, 0, sizeof(*t));
    t->memdata = malloc(memdatasize);
    if (!t->memdata) {
        free(t);
        return NULL;
    }
    t->format = malloc(strlen(format)+1);
    if (!t->format) {
        free(t->memdata);free(t);
        return NULL;
    }
    strcpy(t->format, format);
    memcpy(t->memdata, memdata, memdatasize);
    t->memdatasize = memdatasize;
    t->maxsizex = maxwidth; t->maxsizey = maxheight;
    startthread(t);
    return t;
}

int img_CheckSuccess(void* handle) {
    if (!handle) {return 1;}
    struct loaderthreadinfo* i = handle;
#ifdef WIN
    if (WaitForSingleObject(i->threadhandle, 0) == WAIT_OBJECT_0) {
#else
    pthread_mutex_lock(&i->threadeventmutex);
    int u = i->threadeventobject;
    pthread_mutex_unlock(&i->threadeventmutex);
    if (u == 1) {
#endif
        //request is no longer running
        return 1;
    }else{
        return 0;
    }
}

void img_GetData(void* handle, char** path, int* imgwidth, int* imgheight, char** imgdata) {
    if (!handle) {
        *imgwidth = 0;
        *imgheight = 0;
        *imgdata = NULL;
        *path = NULL;
        //*imgdatasize = 0;
        return;
    }
    struct loaderthreadinfo* i = handle;
    *imgwidth = i->imagewidth;
    *imgheight = i->imageheight;
    *imgdata = i->data;
    if (path) {
        if (i->path) {
            *path = strdup(i->path);
        }else{
            *path = NULL;
        }
    }
    //*imgdatasize = i->datasize; // can be calculated from width*height*4
}

void img_FreeHandle(void* handle) {
    if (!handle) {return;}
    struct loaderthreadinfo* i = handle;
    if (i->memdata) {free(i->memdata);}
    if (i->format) {free(i->format);}
    if (i->path) {free(i->path);}
    //if (i->data) {free(i->data);} //the user needs to do that!
#ifdef WIN
    CloseHandle(i->threadhandle);
#else
    //should be done by pthread_exit()
#endif
    free(handle);
}

static void img_Convert(char* data, int datasize, unsigned char firstchannelto, unsigned char secondchannelto, unsigned char thirdchannelto, unsigned char fourthchannelto) {
    if (!data) {return;}
    int r = 0;
    while (r < datasize) {
        char oldlayout[4];
        memcpy(oldlayout, data + r, 4);
        *(char*)(data + r + firstchannelto) = oldlayout[0];
        *(char*)(data + r + secondchannelto) = oldlayout[2];
        *(char*)(data + r + thirdchannelto) = oldlayout[2];
        *(char*)(data + r + fourthchannelto) = oldlayout[3];
        r += 4;
    }
}

void img_ConvertRGBAtoBGRA(char* imgdata, int datasize) {
    img_Convert(imgdata, datasize, 2, 1, 0, 3);
}

//Simple linear scaler:
void img_Scale(int bytesize, char* imgdata, int originalwidth, int originalheight, char** newdata, int targetwidth, int targetheight) {
    if (*newdata == NULL) {
        *newdata = malloc(targetwidth*targetheight*bytesize);
    }
    if (!(*newdata)) {return;}
    int r,k;
    r = 0;
    float scalex = ((float)targetwidth/(float)originalwidth);
    float scaley = ((float)targetheight/(float)originalheight);
    while (r < targetwidth) {
        k = 0;
        while (k < targetheight) {
            int fromx = r * scalex;
            if (fromx < 0) {fromx = 0;}
            if (fromx >= originalwidth) {fromx = originalwidth;}
            int fromy = k * scaley;
            if (fromy < 0) {fromy = 0;}
            if (fromy >= originalheight) {fromy = originalheight;}
            
            memcpy((*newdata) + (r + (k * targetheight)) * bytesize, imgdata + (fromx + (fromy * originalwidth)) * 4, 4);
            
            k++;
        }
        r++;
    }
}

void img_4to3channel(char* imgdata, int width, int height, char** newdata, int channeltodrop) {
    *newdata = NULL;
    //FIXME
}

