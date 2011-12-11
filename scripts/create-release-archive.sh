#!/bin/bash

if [ -z "$1" ]; then
    echo "You need to specify a release name, e.g. 1.0."
    exit
fi

BINRELEASENAME="linux"
RELEASEVERSION=$1

TARGETHOST=""
if [ $2 == "linux-to-win" ]; then
	echo "Building with MinGW cross compiler... (Please set TARGETHOST in this script properly!)";
	TARGETHOST="i686-pc-mingw32"
	BINRELEASENAME="win32"
else
	echo "Building with native compiler";
fi

rm blitwizard-$RELEASEVERSION-src.zip
rm blitwizard-$RELEASEVERSION-$BINVERSIONNAME.zip
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
zip -r -9 ./blitwizard-$RELEASEVERSION-src.zip ./blitwizard/
cd blitwizard
rm deps.zip
wget http://games.homeofjones.de/blitwizard/deps.zip || { echo "Failed to download deps.zip"; exit 1; }
unzip deps.zip
if [ "$TARGETHOST" == "" ]; then
	./configure || { echo "./configure failed."; exit 1; }
else
	./configure --host="$TARGETHOST" || { echo "./configure failed"; exit 1;}
fi
make
cd ..
mkdir ./blitwizard-bin || { echo "Failed to create blitwizard-bin directory."; exit 1; }
cd blitwizard-bin
mkdir bin
cp ../blitwizard/src/blitwizard* ./bin
cp ../blitwizard/README-libs.txt ./
cp ../blitwizard/Ship-your-game.txt ./
cp -R ../blitwizard/examples ./examples/
cd ..
zip -r -9 ./blitwizard-$RELEASEVERSION-$BINRELEASENAME.zip ./blitwizard-bin/
rm -rf ./blitwizard/
rm -rf ./blitwizard-bin/
