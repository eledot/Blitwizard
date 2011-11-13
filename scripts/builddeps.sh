#!/bin/bash

cd ..

if [ -f scripts/.depsarebuilt ]; then
	currentluatarget=`cat scripts/.luatarget`
	depsluatarget=`cat scripts/.depsarebuilt`
	if [ "$currentluatarget" != "$depsluatarget" ]; then
		echo "Deps will be rebuilt to match new different target.";
	else
        	echo "Deps are already built. To force a rebuild, remove the file scripts/.depsarebuilt";
        	exit;
	fi
fi

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

dir=`pwd`
cd src/imgloader && make deps && make
cd $dir
cd src/vorbis && ./configure && make
cd $dir
cd src/ogg && ./configure && make
cd $dir
cd src/sdl && ./configure --enable-assertions=release --enable-ssemath --disable-pulseaudio --enable-sse2 && make
cd $dir
cp src/sdl/build/.libs/libSDL.a libs/libblitwizardSDL.a
cp src/vorbis/lib/.libs/libvorbis.a libs/libblitwizardvorbis.a
cp src/vorbis/lib/.libs/libvorbisfile.a libs/libblitwizardvorbisfile.a
cp src/ogg/src/.libs/libogg.a libs/libblitwizardogg.a
cp src/imgloader/libimglib.a libs/
cp src/imgloader/libcustompng.a libs/libblitwizardpng.a
cp src/imgloader/libcustomzlib.a libs/libblitwizardzlib.a
cp src/lua/src/liblua.a libs/libblitwizardlua.a

echo "`cat scripts/.luatarget`" > scripts/.depsarebuilt

