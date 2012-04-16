
# Makefile for blit wizard's image loading code

# Uses zlib/libpng.
# To use different versions of zlib or libpng, simply extract their source to the "png" or "zlib" folders.
# libpng versions known to work are: 1.5.5

FILES:=pngloader.o imgloader.o
SSEFLAGS:=`cd ../../ && sh scripts/checksse.sh && cd src/imgloader/`
CFLAGS:=-O2 -fomit-frame-pointer -ffast-math $(SSEFLAGS) -I./zlib/ -I. -DDEBUGLOG
CROSSCOMPILER:=i686-pc-mingw32-gcc
EMPTY:=
ifeq ($(AR),$(EMPTY))
	AR:=ar
endif

comp: clean-objs $(FILES)
	rm -f ./libimglib.a
	$(AR) rcs ./libimglib.a $(FILES)
deps-png:
	yes n | cp -i png/scripts/pnglibconf.h.prebuilt png/pnglibconf.h
	-mv png/pngvalid.c png/pngvalid.c.skip
	cd png && rm -rf ./*.o && $(CC) -I./ -I../zlib/ -c $(CFLAGS) -DPNG_STATIC *.c && rm -rf ./*test*.o
	rm -f ./libcustompng.a
	$(AR) rcs libcustompng.a png/*.o
deps-zlib:
	cd zlib && rm -rf ./*.o && rm -f ./example.c && rm -rf ./minigzip.c && $(CC) -I./ -I../png/ -c $(CFLAGS) *.c
	rm -f ./libcustomzlib.a
	$(AR) rcs libcustomzlib.a zlib/*.o
clean-objs:
	rm -rf ./*.o
clean: clean-objs
	rm -rf ./*.a
	cd png && make clean
	cd zlib && make clean
