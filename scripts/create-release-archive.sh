#!/bin/bash

#Set the native compiler here
CC="gcc34"

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
if [ "$2" == "linux-to-win" ]; then
	echo "Building with MinGW cross compiler... (Please set TARGETHOST in this script properly!)";
	TARGETHOST="$CROSSCC"
	BINRELEASENAME="win32"
	DOSOURCERELEASE="no"
else
	echo "Building with native compiler";
fi

if [ "$DOSOURCERELEASE" == "yes" ]; then
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
rm .gitignore
rm bin/.gitignore
rm src/sdl/.gitignore
rm src/vorbis/.gitignore
rm src/ogg/.gitignore
rm src/lua/.gitignore
rm src/imgloader/png/.gitignore
rm src/imgloader/zlib/.gitignore
cd ..
rm -r blitwizard/
mv ./tarball ./blitwizard || { echo "Failed to rename tarball -> blitwizard."; exit 1; }
if [ "$DOSOURCERELEASE" == "yes" ]; then
	zip -r -9 ./blitwizard-$RELEASEVERSION-src.zip ./blitwizard/
fi
cd blitwizard
rm deps.zip
wget http://games.homeofjones.de/blitwizard/deps.zip || { echo "Failed to download deps.zip"; exit 1; }
unzip deps.zip
if [ "$TARGETHOST" == "" ]; then
	CC="$CC" ./configure || { echo "./configure failed."; exit 1; }
else
	unset $CC
	./configure --host="$TARGETHOST" || { echo "./configure failed"; exit 1;}
fi
make
cd ..
mkdir ./blitwizard-bin || { echo "Failed to create blitwizard-bin directory."; exit 1; }
cd blitwizard-bin
mkdir bin
cp -R ../blitwizard/bin/samplebrowser ./bin/samplebrowser
cp ../blitwizard/bin/game.lua ./bin/game.lua
cp ../blitwizard/bin/blitwizard* ./bin
cp ../blitwizard/README-libs.txt ./
cp ../blitwizard/Ship-your-game.txt ./
cp ../blitwizard/README.txt ./
unix2dos ./README*.txt || { echo "unix2dos not available."; exit 1; }
cp -R ../blitwizard/examples ./examples/
cd ..
zip -r -9 ./blitwizard-$RELEASEVERSION-$BINRELEASENAME.zip ./blitwizard-bin/
rm -rf ./blitwizard/
rm -rf ./blitwizard-bin/
