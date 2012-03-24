
# blitwizard Makefile.am

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = blitwizard

blitwizard_SOURCES = audio.c audiomixer.c audiosourcefadepanvol.c audiosourceffmpeg.c audiosourcefile.c audiosourceloop.c audiosourceogg.c audiosourceresample.c file.c filelist.c graphics.c hash.c hashtable.c library.c luafuncs.c luafuncs_physics.c luastate.c main.c mathhelpers.c osinfo.c physics.cpp threading.c timefuncs.c win32console.c speex/resample.c
blitwizard_LDADD = -limglib -lblitwizardSDL -lblitwizardpng -lblitwizardzlib -lblitwizardvorbisfile -lblitwizardvorbis -lblitwizardogg -lblitwizardlua -lblitwizardbox2d
blitwizard_LDFLAGS = 

AM_CFLAGS = `cat speex/flags.txt` -fno-PIC -O2 -Wall -Wextra -g -fomit-frame-pointer -Isdl/include/ -Iimgloader/ -L../libs -Ilua/src/ -Ispeex/ -Ivorbis/include/ -Iogg/include/ -Iffmpeg/
AM_CXXFLAGS = -fno-PIC -O2 -Wall -Wextra -g -fno-exceptions -fomit-frame-pointer -Isdl/include/ -Iimgloader/ -L../libs -Ilua/src/ -Ivorbis/include/ -Iogg/include/ -Iffmpeg/ -Ibox2d/

# -fno-PIC is only added for some extra speed, same as -O2.

all-local:
	-mv ./blitwizard ../bin/
	-mv ./blitwizard.exe ../bin/
