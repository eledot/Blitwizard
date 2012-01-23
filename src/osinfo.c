
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

#include <stdio.h>
#include "osinfo.h"
#include "file.h"
#include <ctype.h>
#include <string.h>
#include <unistd.h>

static char versionbuf[512] = "";

#ifdef WIN
#include <windows.h>
const char* osinfo_GetSystemName() {
	return "Windows";
}
const char* osinfo_GetSystemVersion() {
	if (strlen(versionbuf) > 0) {return versionbuf;}
	OSVERSIONINFO osvi;
	memset(&osvi, 0, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);
	
	snprintf(versionbuf,sizeof(versionbuf),"%d.%d",(int)osvi.dwMajorVersion,(int)osvi.dwMinorVersion);
	versionbuf[sizeof(versionbuf)-1] = 0;
	return versionbuf;
}
#else

#if defined(__APPLE__) && defined(__MACH__)
#include <CoreServices/CoreServices.h>
#endif
#if defined(__linux) || defined(linux)
#include <sys/utsname.h>
#endif

#if defined(__linux) || defined(linux)
int filecontains(const char* file, const char* name) {
	char filecontents[64];
	FILE* r = fopen(file, "r");
	if (r) {
		int i = fread(filecontents,1,sizeof(filecontents)-1,r);
       	fclose(r);
		if (i > 0) {
			if (i > (int)sizeof(filecontents)-1) {
				i = (int)sizeof(filecontents)-1;
			}
			filecontents[i] = 0;
			if (i < (int)strlen(name)) {return 0;}
			int j = 0;
			while (j < i) {
				int r = j;
				while (r < i && r < (int)strlen(name)+j) {
					if (toupper(filecontents[r]) != toupper(name[r-j])) {
						return 0;
					}
					r++;
				}
				j++;
			}
			return 1;
		}
	}
	return 0;
}
static char distribuf[64] = "";
static const char* osinfo_GetDistributionName() {
	if (strlen(distribuf) > 0) {return distribuf;}
	if (file_DoesFileExist("/etc/redhat-release")) {
		if (filecontains("/etc/redhat-release", "Fedora")) {
			strcpy(distribuf, "Fedora");
		}
		if (filecontains("/etc/redhat-release", "Red Hat")) {
			strcpy(distribuf, "Red Hat");
		}
	}
	if (filecontains("/etc/SuSE-release", "SUSE")) {
		strcpy(distribuf, "SUSE");
	}
	if (filecontains("/etc/SUSE-release", "SUSE")) {
		strcpy(distribuf, "SUSE");
	}	
	if (file_DoesFileExist("/etc/debian_version")) {
		//check for derivates:
		if (filecontains("/etc/lsb-release", "Ubuntu")) {
			strcpy(distribuf, "Ubuntu");
		}
		if (filecontains("/etc/lsb-release", "Mint")) {
			strcpy(distribuf, "LinuxMint");
		}
		//otherwise, assume debian
		if (strlen(distribuf) <= 0) {
			strcpy(distribuf, "Debian");
		}
	}
	if (filecontains("/etc/mandrake-release", "Mandrake")) {
		strcpy(distribuf,"Mandrake");
	}
	if (strlen(distribuf) <= 0) {
		strcpy(distribuf,"Generic");
	}
	return distribuf;
}
#endif

const char* osinfo_GetSystemVersion() {
	//print out detailed system version
	if (strlen(versionbuf) > 0) {return versionbuf;}
#if defined(__APPLE__) && defined(__MACH__)
	//Mac OS X:
	SInt32 majorVersion,minorVersion,bugfixVersion;

	Gestalt(gestaltSystemVersionMajor, &majorVersion);
	Gestalt(gestaltSystemVersionMinor, &minorVersion);
	Gestalt(gestaltSystemVersionBugFix, &bugfixVersion);
	
	snprintf(versionbuf,sizeof(versionbuf),"%d.%d.%d",majorVersion,minorVersion,bugfixVersion);
        versionbuf[sizeof(versionbuf)-1] = 0;
        return versionbuf;
#endif
#if defined(__linux) || defined(linux)
	//Linux:
	struct utsname b;
	memset(&b, 0, sizeof(b));
	if (uname(&b) != 0) {
		strcpy(versionbuf, "Unknown");
		return versionbuf;
	}
	snprintf(versionbuf,sizeof(versionbuf),"%s/%s",osinfo_GetDistributionName(),b.release);
	versionbuf[sizeof(versionbuf)-1] = 0;
	return versionbuf;
#endif
	//All others:
	strcpy(versionbuf,"unknown");
	return versionbuf;
}

static char osbuf[64] = "";
const char* osinfo_GetSystemName() {
	if (strlen(osbuf) > 0) {return osbuf;}
#if defined(__APPLE__) && defined(__MACH__)
	strcpy(osbuf, "Mac OS X");
#endif
#if defined(__FreeBSD__)
	strcpy(osbuf, "FreeBSD");
#endif
#if defined(__NetBSD__)
	strcpy(osbuf, "NetBSD");
#endif
#if defined(__OpenBSD__)
	strcpy(osbuf, "OpenBSD");
#endif
#if defined(__linux) || defined(linux)
	strcpy(osbuf, "Linux");
#endif
#if defined(sun) || defined(__sun)
	strcpy(osbuf, "Solaris");
#endif
#if defined(__BEOS__)
	strcpy(osbuf, "BeOS");
#endif
#if defined(__QNX__)
	strcpy(osbuf, "FreeBSD");
#endif
	if (strlen(versionbuf) <= 0) {
		strcpy(osbuf,"Unknown Unix/POSIX");
	}
	return osbuf;
}
#endif
