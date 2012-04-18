
# blitwizard Makefile.am

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = blitwizard

blitwizard_SOURCES = audio.c audiomixer.c audiosourcefadepanvol.c audiosourceffmpeg.c audiosourcefile.c audiosourceloop.c audiosourceogg.c audiosourceprereadcache.c audiosourceresample.c connections.c file.c filelist.c graphics.c hash.c hashtable.c hostresolver.c ipcheck.c library.c luafuncs.c luafuncs_net.c luafuncs_physics.c luastate.c main.c mathhelpers.c osinfo.c physics.cpp threading.c timefuncs.c win32console.c sockets.c
blitwizard_LDADD = -limglib -lblitwizardSDL -lblitwizardpng -lblitwizardzlib -lblitwizardvorbisfile -lblitwizardvorbis -lblitwizardogg -lblitwizardlua -lblitwizardbox2d
blitwizard_LDFLAGS = 

AM_CFLAGS =  -fno-PIC -O2 -Wall -Wextra -g -fomit-frame-pointer -L../libs/
AM_CXXFLAGS = -fno-PIC -O2 -Wall -Wextra -g -fno-exceptions -fomit-frame-pointer -L ../libs/

# -fno-PIC is only added for some extra speed, same as -O2.

all-local: mytarget
mytarget: blitwizard$(EXEEXT)
	-mv ./blitwizard ../bin/
	-mv ./blitwizard.exe ../bin/
