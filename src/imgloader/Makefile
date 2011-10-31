
# Makefile for blit wizard's image loading code

# Uses zlib/libpng.
# To use different versions of zlib or libpng, simply extract their source to the "png" or "zlib" folders.
# libpng versions known to work are: 1.5.5

FILES:=pngloader.o imgloader.o
CFLAGS:=-O3 -fomit-frame-pointer -ffast-math -msse -mfpmath=sse -I./zlib/ -DDEBUGLOG
CROSSCOMPILER:=i686-pc-mingw32-gcc

comp: clean-objs $(FILES)
	$(AR) rcs ./libimglib$(WIN).a $(FILES)
win:
	make comp WIN="_w32"
deps:
	cd png && rm -rf ./*.o // $(CC) -I./ -I../zlib/ -O2 -fomit-frame-pointer -ffast-math -msse -mfpmath=sse -DPNG_STATIC *.c
	$(AR) rcs libcustompng$(WIN).a png/*.o
	cd zlib && rm -rf ./*.o && $(CC) -I./ -I../png/ -O2 -fomit-frame-pointer -ffast-math -msse -mfpmath=sse *.c
	$(AR) rcs libcustomzlib$(WIN).a zlib/*.o
depswin:
	make deps WIN="_w32"
clean-objs:
	rm -rf ./*.o
clean: clean-objs
	rm -rf ./*.a
	cd png && make clean
	cd zlib && make clean
