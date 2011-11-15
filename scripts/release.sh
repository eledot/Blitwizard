#!/bin/bash

TARGETHOST=""
if [ "$1" == "linux-to-win" ]; then
	TARGETHOST="i686-pc-mingw32"
fi

rm blitwizard-src.zip
rm blitwizard-win32.zip
mkdir tarball
cd tarball
git clone http://games.homeofjones.de/blitwizard/git-source/blitwizard.git/ .
rm -rf ./.git
sh autogen.sh
rm .gitignore
rm bin/.gitignore
rm src/sdl/.gitignore
rm src/vorbis/.gitignore
rm src/ogg/.gitignore
rm src/lua/.gitignore
rm src/imgloader/png/.gitignore
rm src/imgloader/zlib/.gitignore
cd ..
mv ./tarball ./blitwizard || { echo "Failed to rename tarball -> blitwizard."; return 1; }
zip -r -9 ./blitwizard-src.zip ./blitwizard/
cd blitwizard
wget http://games.homeofjones.de/blitwizard/deps.zip
unzip deps.zip
if [ "$TARGETHOST" == "" ]; then
	./configure
else
	./configure --host="$TARGETHOST"
fi
make
cd ..
mkdir ./blitwizard-bin || { echo "Failed to create blitwizard-bin directory."; return 1; }
cd blitwizard-bin
mkdir bin
cp ../blitwizard/src/blitwizard.exe ./bin
cp ../blitwizard/README-libs.txt ./
cp ../blitwizard/Ship-your-game.txt ./
cp -R ../blitwizard/examples ./examples/
cd ..
zip -r -9 ./blitwizard-win32.zip ./blitwizard-bin/
rm -rf ./blitwizard/
rm -rf ./blitwizard-bin/
