
# blitwizard Makefile.am

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = blitwizard

blitwizard_SOURCES = audio.c audiomixer.c audiosourcefadepanvol.c audiosourceffmpeg.c audiosourcefile.c audiosourceloop.c audiosourceogg.c audiosourceresample.c file.c graphics.c hash.c hashtable.c library.c luafuncs.c luastate.c main.c osinfo.c timefuncs.c win32console.c
blitwizard_LDADD = -limglib -lblitwizardSDL -lblitwizardpng -lblitwizardzlib -lblitwizardvorbisfile -lblitwizardvorbis -lblitwizardogg -lblitwizardlua
blitwizard_LDFLAGS = 

AM_CFLAGS = -fno-PIC -O2 -Wall -Wextra -g -fomit-frame-pointer -Wl,-Bsymbolic -Isdl/include/ -Iimgloader/ -L../libs -Ilua/src/ -Ivorbis/include/ -Iogg/include/ -Iffmpeg/
# -fno-PIC is only added for some extra speed, same as -O2.

all-local:
	-mv ./blitwizard ../bin/
	-mv ./blitwizard.exe ../bin/
