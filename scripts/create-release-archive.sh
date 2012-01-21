#!/bin/bash

#Set the native compiler here
CC="gcc"

#Set the cross compilation toolchain name here
CROSSCCHOST="i686-pc-mingw32"

if [ -z "$1" ]; then
    echo "You need to specify a release name, e.g. 1.0."
    exit
fi

BINRELEASENAME="linux"
RELEASEVERSION=$1
DOSOURCERELEASE="yes"

TARGETHOST=""
if [ "$2" = "linux-to-win" ]; then
	echo "Building with MinGW cross compiler... (Please set CROSSCCHOST in this script properly!)";
	TARGETHOST="$CROSSCCHOST"
	BINRELEASENAME="win32"
	DOSOURCERELEASE="no"
else
	echo "Building with native compiler";
	if [ "$2" = "win" ]; then
		BINRELEASENAME="win32"
	fi
fi

if [ "$DOSOURCERELEASE" = "yes" ]; then
	rm blitwizard-$RELEASEVERSION-src.zip
fi
rm blitwizard-$RELEASEVERSION-$BINRELEASENAME.zip
rm deps.zip
rm -r tarball/
mkdir -p tarball
cd tarball
git clone http://games.homeofjones.de/blitwizard/git-source/blitwizard.git/ . || { echo "Failed to do git checkout."; exit 1; }
rm -rf ./.git
sh autogen.sh || { echo "Autoconf generation failed."; exit 1; }

# Remove things from source release we don't want
rm .gitignore
rm bin/.gitignore
rm src/sdl/.gitignore
rm src/vorbis/.gitignore
rm src/ogg/.gitignore
rm src/lua/.gitignore
rm src/imgloader/png/.gitignore
rm src/imgloader/zlib/.gitignore
cd ..

# Rename to ./blitwizard/ and zip for source release
rm -r blitwizard/
mv ./tarball ./blitwizard || { echo "Failed to rename tarball -> blitwizard."; exit 1; }
if [ "$DOSOURCERELEASE" = "yes" ]; then
	mv ./blitwizard/ffmpeg-*.tar.bz2 ./
	zip -r -9 ./blitwizard-$RELEASEVERSION-src.zip ./blitwizard/ || { mv ./ffmpeg-*.tar.bz2 ./blitwizard/; echo "zip doesn't work as expected - is zip installed?"; exit 1; }
	mv ./ffmpeg-*.tar.bz2 ./blitwizard/
fi
cd blitwizard

# Get dependencies
rm deps.zip
wget http://games.homeofjones.de/blitwizard/deps.zip || { echo "Failed to download deps.zip for dependencies"; exit 1; }
unzip deps.zip

# Do ./configure
if [ "$TARGETHOST" = "" ]; then
	CC="$CC" ./configure || { echo "./configure failed."; exit 1; }
else
	unset $CC
	./configure --host="$TARGETHOST" || { echo "./configure failed"; exit 1; }
fi

# Ensure FFmpeg support by providing the source of it
mkdir src/ffmpeg/
cd src/ffmpeg/
tar -xvjf --strip-components=1 ../../ffmpeg-*.tar.bz2 ./

# Compile
make || { echo "Compilation failed."; exit 1; }
cd ..
rm -r blitwizard-bin
mkdir ./blitwizard-bin || { echo "Failed to create blitwizard-bin directory."; exit 1; }
cd blitwizard-bin
mkdir bin

# Copy all the files we want for a binary distribution
cp -R ../blitwizard/bin/samplebrowser ./bin/samplebrowser
cp -R ../blitwizard/templates ./templates
cp ../blitwizard/bin/game.lua ./bin/game.lua
cp ../blitwizard/bin/blitwizard* ./bin
cp ../blitwizard/README-libs.txt ./
cp ../blitwizard/README-ffmpeg.txt ./
cp ../blitwizard/README-lgpl.txt ./
cp ../blitwizard/Ship-your-game.txt ./
cp ../blitwizard/README.txt ./

if [ -e "./bin/blitwizard.exe" ]; then
	# Add easy start script
	cp ../blitwizard/Run-Blitwizard.bat ./
	
	# Add FFmpeg to windows distribution
	cp ../../src/ffmpeg/libavformat/avformat.dll ./avformat.dll
    cp ../../src/ffmpeg/libavutil/avutil.dll ./avutil.dll
    cp ../../src/ffmpeg/libavcodec/avcodec.dll ./avcodec.dll
	cp ../blitwizard/ffmpeg-*.tar.bz2 ./
fi

# detect todos or unix2dos
UNIXTODOS="unix2dos"
$UNIXTODOS ./README*.txt || {
	UNIXTODOS="todos"
	$UNIXTODOS ./README*.txt || { echo "unix2dos/todos not working - is any of them installed?"; exit 1; }
}
$UNIXTODOS ./Ship-your-game.txt
cp -R ../blitwizard/examples ./examples/

$UNIXTODOS ./examples/01.helloworld/*.lua
$UNIXTODOS ./examples/02.simplecar/*.lua
$UNIXTODOS ./examples/03.sound/*.lua
$UNIXTODOS ./examples/04.simplecar.async/*.lua

cd ..
zip -r -9 ./blitwizard-$RELEASEVERSION-$BINRELEASENAME.zip ./blitwizard-bin/
rm -rf ./blitwizard/
rm -rf ./blitwizard-bin/
