#!/bin/bash

cd ..

luatarget=`cat scripts/.luatarget | grep luatarget | sed -e 's/^luatarget\=//'`
changedhost=""

if [ -f scripts/.depsarebuilt ]; then
	depsluatarget=`cat scripts/.depsarebuilt`
	if [ "$luatarget" != "$depsluatarget" ]; then
		echo "Deps will be rebuilt to match new different target.";
		changedhost="yes";
	else
        	echo "Deps are already built. To force a rebuild, remove the file scripts/.depsarebuilt";
        	exit;
	fi
fi

CC=`cat scripts/.luatarget | grep CC | sed -e 's/^.*\=//'`
RANLIB=`cat scripts/.luatarget | grep RANLIB | sed -e 's/^.*\=//'`
AR=`cat scripts/.luatarget | grep AR | sed -e 's/^.*\=//'`
HOST=`cat scripts/.luatarget | grep HOST | sed -e 's/^.*\=//'`
MACBUILD=`cat scripts/.luatarget | grep MACBUILD | sed -e 's/^.*\=//'`

if [ ! -f src/imgloader/png/png.c ]; then
	echo "MISSING DEPENDENCY: Please extract the contents of a recent libpng tarball into src/imgloader/png/ - or read README.txt";
	exit;
fi
if [ ! -f src/imgloader/zlib/gzlib.c ]; then
        echo "MISSING DEPENDENCY: Please extract the contents of a recent zlib tarball into src/imgloader/zlib/ - or read README.txt";
	exit;
fi
if [ ! -f src/sdl/src/SDL.c ]; then
        echo "MISSING DEPENDENCY: Please extract the contents of a recent SDL 1.3 tarball into src/sdl/ - or read README.txt";
	exit;
fi
if [ ! -f src/vorbis/lib/vorbisenc.c ]; then
        echo "MISSING DEPENDENCY: Please extract the contents of a recent libvorbis tarball into src/vorbis/ - or read README.txt";
	exit;
fi
if [ ! -f src/ogg/src/framing.c ]; then
        echo "MISSING DEPENDENCY: Please extract the contents of a recent libogg tarball into src/ogg/ - or read README.txt";
	exit;
fi
if [ ! -f src/lua/src/lua.h ]; then
        echo "MISSING DEPENDENCY: Please extract the contents of a recent Lua 5 tarball into src/lua/ - or read README.txt";
        exit;
fi

export CC="$CC"
export AR="$AR"

dir=`pwd`

# This will build libpng/zlib and our custom threaded image loader wrapper
cd src/imgloader && make deps && make || { echo "Failed to compile libpng/zlib/imgloader"; exit 1; }
cd $dir

# Build ogg
cd src/ogg && ./configure --host="$HOST" --disable-shared --enable-static && make clean && make || { echo "Failed to compile libogg"; exit 1; }
cd $dir

# Build vorbis and remember to tell it where ogg is
oggincludedir="`pwd`/src/ogg/include/"
ogglibrarydir="`pwd`/src/ogg/src/.libs/"
if [ "MACBUILD" != "yes" ]; then
	cd src/vorbis && ./configure --host="$HOST" --with-ogg-libraries="$ogglibrarydir" --with-ogg-includes="$oggincludedir" --disable-oggtest --disable-docs --disable-examples --disable-shared --enable-static && make clean && make || { echo "Failed to compile libvorbis"; exit 1; }
else
	cd src/vorbis && ./configure --with-ogg-libraries="$ogglibrarydir" --with-ogg-includes="$oggincludedir" --disable-oggtest --enable-static && make clean && make || { echo "Failed to compile libvorbis"; exit 1; }
fi
cd $dir

# Build SDL 1.3
cd src/sdl && ./configure --host="$HOST" --enable-assertions=release --enable-ssemath --disable-pulseaudio --enable-sse2 --disable-shared --enable-static || { echo "Failed to compile SDL 1.3"; exit 1; }
cd $dir
if [ "changeddeps" == "yes" ]; then
	cd src/sdl && make clean || { echo "Failed to compile SDL 1.3"; exit 1; }
	cd $dir
fi
cd src/sdl && make || { echo "Failed to compile SDL 1.3"; exit 1; }
cd $dir

# Avoid the overly stupid Lua build script which doesn't even adhere to $CC
cd src/lua/ && rm -f src/liblua.a && rm -rf build/
cd $dir
cd src/lua/ && mkdir -p build/ && cp src/*.c build/ && cp src/*.h build/
cd $dir
cd src/lua/build && rm luac.c && $CC -c -O2 *.c && $AR rcs ../src/liblua.a *.o || { echo "Failed to compile Lua 5"; exit 1; }
cd $dir

# Wipe out the object files of blitwizard if we need to
if [ "$changedhost" == "yes" ]; then
	rm -f src/*.o
fi

# Copy libraries
cp src/sdl/build/.libs/libSDL.a libs/libblitwizardSDL.a
cp src/vorbis/lib/.libs/libvorbis.a libs/libblitwizardvorbis.a
cp src/vorbis/lib/.libs/libvorbisfile.a libs/libblitwizardvorbisfile.a
cp src/ogg/src/.libs/libogg.a libs/libblitwizardogg.a
cp src/imgloader/libimglib.a libs/
cp src/imgloader/libcustompng.a libs/libblitwizardpng.a
cp src/imgloader/libcustomzlib.a libs/libblitwizardzlib.a
cp src/lua/src/liblua.a libs/libblitwizardlua.a

# Remember for which target we built
echo "$luatarget" > scripts/.depsarebuilt

