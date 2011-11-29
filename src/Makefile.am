
# blitwizard Makefile.am

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = blitwizard

blitwizard_SOURCES = audio.c audiosourcefile.c audiosourceogg.c file.c graphics.c hash.c luafuncs.c luastate.c main.c osinfo.c timefuncs.c
blitwizard_LDADD = -limglib -lblitwizardSDL -lblitwizardpng -lblitwizardzlib -lblitwizardvorbisfile -lblitwizardvorbis -lblitwizardogg -lblitwizardlua
blitwizard_LDFLAGS = 

AM_CFLAGS = -fno-PIC -O2 -Wall -Wextra -Wno-sign-compare -g -fno-stack-protector -Wl,-Bsymbolic -Isdl/include/ -Iimgloader/ -L../libs -Ilua/src/
# The -fno-stack-protector option is only put here for increased binary compatibility with some older Linux systems:
# -fno-PIC is only added for some extra speed, same as -O2.

all-local:
	cp ./blitwizard* ../bin/
