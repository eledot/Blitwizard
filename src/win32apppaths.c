
/* blitwizard game engine - source code file

  Copyright (C) 2012-2013 Nicolas Haunold, Jonas Thiem

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
#include <windows.h>
#include "os.h"
#include "file.h"

static char* queryregstring(HKEY key, const char* path, const char* name) {
    // the windows registry knows where Steam is:
    char data[256] = "";
    unsigned int datalen = sizeof(data) - 1;
    HKEY khandle;
    if (RegOpenKeyEx(key, path, 0, KEY_QUERY_VALUE, &khandle) != ERROR_SUCCESS) {
        return NULL;
    }
    if (RegQueryValueEx(khandle, name, 0, 0, (LPBYTE)data, (DWORD*)&datalen) != ERROR_SUCCESS) {
        RegCloseKey(khandle);
        return NULL;
    }

    // null terminate the queried value properly:
    if (datalen >= sizeof(data)) {
        datalen = sizeof(data) - 1;
    }
    data[datalen] = 0;
    RegCloseKey(khandle);
    return strdup(data);
}

char steampath[512] = "";
const char* win32_GetPathForSteam() {
    if (steampath[0] != '\0') {
        return steampath;
    }

    // the registry knows where steam is:
    char* path = queryregstring(HKEY_CURRENT_USER, "Software\\\\Valve\\\\Steam", "SteamPath");
    if (!path) {
        return NULL;
    }
    file_MakeSlashesNative(path);

    // copy and remember the path:
    unsigned int copylen = strlen(path);
    if (copylen >= sizeof(steampath)) {
        copylen = sizeof(steampath) - 1;
    }
    memcpy(steampath, path, copylen);
    steampath[copylen] = 0;
    free(path);

    return steampath;
}

char chromepath[512] = "";
const char* win32_GetPathForChrome() {
    if (chromepath[0] != '\0') {
        return chromepath;
    }

    // try the app paths for chrome:
    char* path = queryregstring(HKEY_LOCAL_MACHINE, "Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\App Paths\\chrome.exe", "Path");
    if (!path) {
        return NULL;
    }

    file_MakeSlashesNative(path); // now: C:\Program Files (x86)\Google\Chrome\Application
    // copy and remember the path:
    unsigned int copylen = strlen(path);
    if (copylen >= sizeof(chromepath)) {
        copylen = sizeof(chromepath) - 1;
    }
    memcpy(chromepath, path, copylen);
    chromepath[copylen] = 0;
    free(path);

    return chromepath;
}


