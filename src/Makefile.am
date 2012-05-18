
# blitwizard Makefile.am

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = blitwizard

blitwizard_SOURCES = audio.c audiomixer.c audiosourcefadepanvol.c audiosourceffmpeg.c audiosourceflac.c audiosourcefile.c audiosourceformatconvert.c audiosourceloop.c audiosourceogg.c audiosourceprereadcache.c audiosourceresample.c connections.c file.c filelist.c graphics.c graphicssdl.c graphicsnull.c graphicstexturelist.c hash.c hashtable.c hostresolver.c ipcheck.c library.c listeners.c luaerror.c luafuncs.c luafuncs_net.c luafuncs_physics.c luastate.c main.c mathhelpers.c osinfo.c physics.cpp threading.c timefuncs.c win32console.c sockets.c
blitwizard_LDADD = 
blitwizard_LDFLAGS = $(FINAL_LD_FLAGS)

AM_CFLAGS = $(FINAL_INCLUDE_FLAGS) -fno-PIC -O2 -fno-strict-aliasing -Wall -Wextra -g -fomit-frame-pointer -L../libs/ $(FINAL_USE_LIB_FLAGS)
AM_CXXFLAGS = $(FINAL_INCLUDE_FLAGS) $(STATICSTDCXX) -fno-PIC -O2 -fno-strict-aliasing -Wall -Wextra -g -fno-exceptions -fomit-frame-pointer -L ../libs/ $(FINAL_USE_LIB_FLAGS)

# -fno-PIC is only added for some extra speed, same as -O2.

all-local: mytarget
mytarget: blitwizard$(EXEEXT)
	sh ../scripts/copybin.sh
