
# blitwizard Makefile.am

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = blitwizard

blitwizard_SOURCES = audio.c audiomixer.c audiosourcefadepanvol.c audiosourceffmpeg.c audiosourceflac.c audiosourcefile.c audiosourceformatconvert.c audiosourceloop.c audiosourceogg.c audiosourceprereadcache.c audiosourceresample.c connections.c file.c filelist.c graphics.c graphics2d3d.cpp graphics2d3drender.cpp graphicsnull.c graphicstexturelist.c hash.c hashtable.c hostresolver.c ipcheck.c library.c listeners.c logging.c luaerror.c luafuncs.c luafuncs_net.c luafuncs_media_object.c luafuncs_object.c luafuncs_objectgraphics.c luafuncs_objectphysics.c luastate.c main.c mathhelpers.c osinfo.c physics2d.cpp threading.c timefuncs.c win32console.c sockets.c
blitwizard_LDADD = 
blitwizard_LDFLAGS = $(FINAL_LD_FLAGS)

if MAC_APP_PATHS
blitwizard_SOURCES += macapppaths.m
endif
if WIN_APP_PATHS
blitwizard_SOURCES += win32apppaths.c
endif

AM_CFLAGS = $(FINAL_INCLUDE_FLAGS) -fno-PIC -O3 -DSYSTEM_TEMPLATE_PATH=\"${datadir}/blitwizard/templates/\" -fno-strict-aliasing -Werror-implicit-function-declaration -Wstrict-prototypes -Wfree-nonheap-object -Wall -Wextra -g -fomit-frame-pointer -L../libs/ $(FINAL_USE_LIB_FLAGS)
AM_CXXFLAGS = $(FINAL_INCLUDE_FLAGS) $(STATICSTDCXX) -fno-PIC -O3 -fno-strict-aliasing -Wall -Wextra -g -fexceptions -fomit-frame-pointer -L ../libs/ $(FINAL_USE_LIB_FLAGS)
AM_OBJCFLAGS = -fno-PIC -Wall -O3 -g -fno-exceptions -fomit-frame-pointer

# -fno-PIC is only added for some extra speed, same as -O2.

all-local: mytarget
mytarget: blitwizard$(EXEEXT)
	sh ../scripts/copybin.sh
install:
	mkdir -p ${datadir}/blitwizard
	rm -rf ${datadir}/blitwizard/templates/
	cp -R ../templates/ ${datadir}/blitwizard/templates/
	chmod -R 775 ${datadir}/blitwizard
	cp ../bin/blitwizard${EXEEXT} ${bindir}
	chmod 755 ${bindir}/blitwizard${EXEEXT}
count:
	cloc *.c *.h imgloader/*.c imgloader/*.h *.cpp
