
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

#include "os.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "filelist.h"
#include "file.h"

#ifdef WINDOWS
#include <windows.h>
#else
#include <stddef.h>
#include <dirent.h>
#include <errno.h>
#endif

struct filelistcontext {
    char* path;
#ifdef WINDOWS
    int reporterror;
    WIN32_FIND_DATA finddata;
    HANDLE findhandle;
#else
    struct dirent* entrybuf;
    DIR* directoryptr;
#endif
};

struct filelistcontext* filelist_Create(const char* path) {
    //get a corrected path from this (slahes done correctly etc)
    char* p = strdup(path);
    if (!p) {
        return NULL;
    }
    file_MakeSlashesNative(p);

    //handle current directory properly:
    if (strcasecmp(p, "") == 0) {
        free(p);
        p = strdup(".");
        if (!p) {
            return NULL;
        }
    }

    //check whether path exists and is a directory
    if (!file_DoesFileExist(p)) {
        free(p);
        return NULL;
    }
    if (!file_IsDirectory(p)) {
        free(p);
        return NULL;
    }

    //get new context
    struct filelistcontext* ctx = malloc(sizeof(*ctx));
    if (!ctx) {
        free(p);
        return NULL;
    }
    memset(ctx, 0, sizeof(*ctx));

    ctx->path = strdup(p);
    if (!ctx->path) {
        free(p);
        free(ctx);
        return NULL;
    }

#ifndef WINDOWS
    //initialise context on Unix
    ctx->directoryptr = opendir(p);
    free(p);
    if (!ctx->directoryptr) {
        free(ctx->path);
        free(ctx);
        return NULL;
    }

    //this size calculation is directly from linux' man 3 readdir:
    size_t len = (offsetof(struct dirent, d_name) +
                 pathconf(ctx->path, _PC_NAME_MAX) + 1 + sizeof(long))
                 & -sizeof(long);

    ctx->entrybuf = malloc(len);
#else
    //initialise context on Windows
    ctx->findhandle = INVALID_HANDLE_VALUE;

    //make sure we have no invalid wildcard chars in the path
    unsigned int i = 0;
    while (i < strlen(ctx->path)) {
        if (ctx->path[i] == '?' || ctx->path[i] == '*') {
            //this is invalid
            free(ctx->path);
            free(ctx);
            return NULL;    
        }
        i++;
    }

    //see if we end in a separator
    char sep[2] = "\\";
    if (strlen(ctx->path) > 0 && ctx->path[strlen(ctx->path)-1] == '\\') {
        strcpy(sep, "");
    }

    //construct search pattern and init search
    char finddir[600];
    snprintf(finddir,599,"%s%s*",ctx->path,sep);
    finddir[599] = 0;
    ctx->findhandle = FindFirstFile(finddir, &ctx->finddata);
    if (ctx->findhandle == INVALID_HANDLE_VALUE) {
        //check if directory is empty:
        if (GetLastError() != ERROR_FILE_NOT_FOUND) {
            //other error:
            free(ctx->path);
            free(ctx);
            return 0;
        }
        //directory empty, so we want to continue but report no files later
    }   
#endif

    return ctx;
}

#ifdef WINDOWS
void filelist_PrepareNextFile(struct filelistcontext* ctx) {
    //prepare the next file we shall examine next time
    if (FindNextFile(ctx->findhandle, &ctx->finddata) == 0) {
        if (GetLastError() == ERROR_NO_MORE_FILES) {
            //prepare for next time to return no files
            FindClose(ctx->findhandle);
            ctx->findhandle = INVALID_HANDLE_VALUE;
        }else{
            //this is an actual error. however, we want to report it next time
            ctx->reporterror = 1;
        }
    }
}
#endif

int filelist_GetNextFile(struct filelistcontext* ctx, char* namebuf, size_t namebufsize, int* isdirectory) {
#ifdef WINDOWS
    char* filename;
#else
    const char* filename;
#endif
    int setisdirectory = 1;
    int originalisdirectoryvalue = 0;
    if (isdirectory) {
        originalisdirectoryvalue = *isdirectory;
    }
#ifndef WINDOWS
    //read next file entry on Unix
    struct dirent* result;
    if (readdir_r(ctx->directoryptr, ctx->entrybuf, &result) != 0) {
        //an error occured.
        return -1;
    }

    //handle end of directory
    if (result == NULL) {
        return 0;
    }

    //check for . and .. which we want to omit
    if (memcmp(ctx->entrybuf->d_name, ".", 2) == 0 || memcmp(ctx->entrybuf->d_name, "..", 3) == 0) {
        //this won't recurse a lot so it should be fine
        return filelist_GetNextFile(ctx, namebuf, namebufsize, isdirectory);
    }

    //possible directory check shortcut
#ifdef _DIRENT_HAVE_D_TYPE
    if (ctx->entrybuf->d_type != DT_UNKNOWN && isdirectory) {
        if (ctx->entrybuf->d_type == DT_DIR) {
            *isdirectory = 1;
        }else{
            *isdirectory = 0;
        }
        setisdirectory = 0;
    }
#endif
    filename = ctx->entrybuf->d_name;

    //obtain the length of the name
    unsigned int length = 0;
#if defined(__APPLE__) && defined(__MACH__)
#define NAME_MAX FILENAME_MAX
#endif
    while (length < NAME_MAX) {
    
        if (ctx->entrybuf->d_name[length] == 0) {
            break;
        }
        length++;
    }
#else //ifndef WINDOWS
    //check if we actually have an empty dir
    if (ctx->findhandle == INVALID_HANDLE_VALUE) {
        if (ctx->reporterror) {
            return -1; //report error we encountered last time
        }
        return 0; //report no file
    }

    //check for . and .. which we want to omit
    if (memcmp(ctx->finddata.cFileName, ".", 2) == 0 || memcmp(ctx->finddata.cFileName, "..", 3) == 0) {
        //this won't recurse a lot so it should be fine
        filelist_PrepareNextFile(ctx);
        return filelist_GetNextFile(ctx, namebuf, namebufsize, isdirectory);
    }

    //retrieve current file name
    filename = strdup(ctx->finddata.cFileName);
    if (!filename) {
        if (isdirectory) {
            //revert value on error so it is unaltered
            *isdirectory = originalisdirectoryvalue;
        }
        return -1;
    }
    unsigned int length = strlen(filename);

    filelist_PrepareNextFile(ctx);
#endif

    //check whether it is a directory, the long way :p
    if (setisdirectory && isdirectory) {
        char* p = file_AddComponentToPath(ctx->path, filename);
        if (!p) {
            if (isdirectory) {
                //revert value on error so it is unaltered
                *isdirectory = originalisdirectoryvalue;
            }
#ifdef WINDOWS
            free(filename);
#endif
            return -1;
        }
        if (file_IsDirectory(p)) {
            *isdirectory = 1;
        }else{
            if (file_DoesFileExist(p)) {
                *isdirectory = 0;
            }else{
                if (isdirectory) {
                    //revert value on error so it is unaltered
                    *isdirectory = originalisdirectoryvalue;
                }
#ifdef WINDOWS
                free(filename);
#endif
                free(p);
                return -1;
            }
        }
        free(p);
    }

    //copy the name
    if (length < namebufsize) {
        memcpy(namebuf, filename, length);
        namebuf[length] = 0;
    }else{
#ifdef WINDOWS
        free(filename);
#endif
        if (isdirectory) {
            //revert value on error so it is unaltered
            *isdirectory = originalisdirectoryvalue;
        }
        return -1;
    }
#ifdef WINDOWS
    free(filename);
#endif
    return 1;
}

void filelist_Free(struct filelistcontext* ctx) {
    free(ctx->path);

#ifndef WINDOWS
    //free file list on Unix
    free(ctx->entrybuf);
    closedir(ctx->directoryptr);
#else
    //free file list on Windows
    if (ctx->findhandle != INVALID_HANDLE_VALUE) {
        FindClose(ctx->findhandle);
    }
#endif
    
    free(ctx);
}

