#!/bin/bash

cd ..

luatarget=`cat scripts/.luatarget | grep luatarget | sed -re 's/^luatarget\=//'`

if [ -f scripts/.depsarebuilt ]; then
	depsluatarget=`cat scripts/.depsarebuilt`
	if [ "$luatarget" != "$depsluatarget" ]; then
		echo "Deps will be rebuilt to match new different target.";
	else
        	echo "Deps are already built. To force a rebuild, remove the file scripts/.depsarebuilt";
        	exit;
	fi
fi

CC=`cat scripts/.luatarget | grep CC | sed -re 's/^.+\=//'`
AR=`cat scripts/.luatarget | grep AR | sed -re 's/^.+\=//'`
HOST=`cat scripts/.luatarget | grep HOST | sed -re 's/^.+\=//'`

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
cd src/imgloader && make deps && make
cd $dir
cd src/ogg && ./configure --host="$HOST" --disable-shared --enable-static && make
cd $dir
oggincludedir="`pwd`/../ogg/src/include/"
ogglibrarydir="`pwd`/../ogg/src/.libs/"
cd src/vorbis && ./configure --host="$HOST" --with-ogg-libraries="$ogglibrarydir" --with-ogg-includes="$oggincludedir" --disable-oggtest --disable-docs --disable-examples --disable-shared --enable-static && make
cd $dir
cd src/sdl && ./configure --host="$HOST" --enable-assertions=release --enable-ssemath --disable-pulseaudio --enable-sse2 --disable-shared --enable-static && make
cd $dir
cd src/lua/ && make $luatarget
cd $dir
cp src/sdl/build/.libs/libSDL.a libs/libblitwizardSDL.a
cp src/vorbis/lib/.libs/libvorbis.a libs/libblitwizardvorbis.a
cp src/vorbis/lib/.libs/libvorbisfile.a libs/libblitwizardvorbisfile.a
cp src/ogg/src/.libs/libogg.a libs/libblitwizardogg.a
cp src/imgloader/libimglib.a libs/
cp src/imgloader/libcustompng.a libs/libblitwizardpng.a
cp src/imgloader/libcustomzlib.a libs/libblitwizardzlib.a
cp src/lua/src/liblua.a libs/libblitwizardlua.a

echo "$luatarget" > scripts/.depsarebuilt

