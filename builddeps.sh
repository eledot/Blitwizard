#!/bin/sh

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
