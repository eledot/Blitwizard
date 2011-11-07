
char versionbuf[512] = "";

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
	
	snprintf(versionbuf,sizeof(versionbuf),"%d.%d",osvi.dwMajorVersion,osvi.dwMinorVersion);
	versionbuf[sizeof(versionbuf)-1] = 0;
	return versionbuf;
}
#else

#endif