
# blitwizard Makefile.am

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = blitwizard

blitwizard_SOURCES = graphics.c hash.c main.c osinfo.c
blitwizard_LDADD = -limglib -lblitwizardSDL -lblitwizardpng -lblitwizardzlib -lblitwizardvorbisfile -lblitwizardvorbis -lblitwizardogg
blitwizard_LDFLAGS = 

AM_CFLAGS = -fno-PIC -O2 -Wall -Wextra -Wno-sign-compare -g -fno-stack-protector -DIPV6 -Wl,-Bsymbolic -Isdl/include/ -Iimgloader/ -L../libs
# The -fno-stack-protector option is only put here for increased binary compatibility with some older Linux systems:
# -fno-PIC is only added for some extra speed, same as -O2.

