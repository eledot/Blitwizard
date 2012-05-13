#!/bin/bash

# -- Native compilation options --
#Set the native compiler here (ignored for Mac):
CC="gcc"

# -- Linux > Android cross compilation options --
#Point this to the Android SDK/NDK folders
ANDROID_SDK_PATH="/home/jonas/d/androidsdk/android-sdk-linux/"
ANDROID_NDK_PATH="/home/jonas/d/androidsdk/android-ndk-r7/"

# -- Linux > Windows cross compilation options --
#Set the cross compilation toolchain name here:
CROSSCCHOST="i686-pc-mingw32"

# -- End of options --


useversion=`grep AC_INIT ../configure.ac | sed -e "s/AC_INIT[(][[]blitwizard[]], [[]//g" | sed -e "s/[]])//g"`

#Output some explanations
echo "This is a release script that builds a blitwizard release tarball."
echo "It has been purely tested and used on Linux so far,"
echo "most likely it won't work anywhere else without major changes."
echo "Usage:"
echo "  create-release-archive.sh [version label] [mode]"
echo "Example:"
echo "  create-release-archive.sh current"
echo "Available modes:"
echo "  [no mode]       Specify no mode to simply compile for native Linux"
echo "  linux-to-win    Compile with a cross compiler for Windows."
echo "                  Edit the script and set CROSSCCHOST properly!"
echo "                  (You need to install a cross compiler for windows"
echo "                  first, all major linux distributions offer one in"
echo "                  their repository)"
echo "  android         Compile with a cross compiler for Android."
echo "                  Install the Android SDK first and the Android NDK."
echo "                  Edit the script and set the ANDROID_ paths properly!"

#Set the native compiler here
CC="gcc"

#Set the cross compilation toolchain name here
CROSSCCHOST="i686-pc-mingw32"

#Point this to the Android SDK folder
ANDROID_SDK_PATH="/home/jonas/d/androidsdk/android-sdk-linux/"
ANDROID_NDK_PATH="/home/jonas/d/androidsdk/android-ndk-r7/"



#Deal with curious users first:
if [ "$1" = "-help" ]; then
    exit 0;
fi
if [ "$1" = "--help" ]; then
    exit 0;
fi
if [ "$1" = "-h" ]; then
    exit 0;
fi

if [ -z "$1" ]; then
    echo "You need to specify a release name, e.g. 1.0."
    exit
fi

if [ ! "$1" = "current" ]; then
    useversion="$1"
fi

BINRELEASENAME="linux"
RELEASEVERSION="$useversion"
DOSOURCERELEASE="yes"

REDOWNLOAD="yes"
RELEASETYPE="$2"
if [ "$RELEASETYPE" = "android" ]; then
    if [ -d "blitwizard-android/" ]; then
        read -p "Redownload source and deps? [y/N]"
        [[ "$REPLY" == [yY] ]] || { REDOWNLOAD="no"; }
    fi
    RELEASETYPE="android"
fi

TARGETHOST=""
if [ "$RELEASETYPE" = "linux-to-win" ]; then
    echo "";
    echo "Building with MinGW cross compiler..."
    echo "IMPORTANT: Please set CROSSCCHOST in this script properly!"
    echo "If you haven't, press CTRL+C now to cancel, then do so."
    echo "   Given cross compiler host: $CROSSCCHOST"
    echo ""
    TARGETHOST="$CROSSCCHOST"
    BINRELEASENAME="win32"
    DOSOURCERELEASE="no"
else
    if [ "$RELEASETYPE" = "android" ]; then
        echo "";
        echo "Building with Android SDK..."
        echo "IMPORTANT: Please set ANDROID_ paths in this script properly!"
        echo "If you haven't, press CTRL+C now to cancel, then do so."
        echo "   Given Android SDK path: $ANDROID_SDK_PATH"
        echo "   Given Android NDK path: $ANDROID_NDK_PATH"
        echo ""
        DOSOURCERELEASE="no"
        BINRELEASENAME="android"
        echo "Checking for $ANDROID_NDK_PATH/ndk-build..."
        if [ ! -e "$ANDROID_NDK_PATH/ndk-build" ]; then
            echo "ERROR. NDK build utility not found. Is the Android NDK path correct?"
            exit 1
        fi
        if [ ! -e "$ANDROID_SDK_PATH/SDK Readme.txt" ]; then
            echo "ERROR. SDK readme not found. Is the Android SDK path correct?"
            exit 1
        fi
    else
        echo "Building with native compiler";
        if [ "$RELEASETYPE" = "win" ]; then
            BINRELEASENAME="win32"
        fi  
    fi
fi

if [ "$REDOWNLOAD" = "yes" ]; then
    # Remove old stuff
    rm -rf blitwizard-android/
    # (Re)download the tarball
    if [ "$DOSOURCERELEASE" = "yes" ]; then
        rm blitwizard-$RELEASEVERSION-source.zip
    fi
    rm blitwizard-$RELEASEVERSION-$BINRELEASENAME.zip
    rm deps.zip
    rm -r tarball/
    mkdir -p tarball
    cd tarball
    git clone https://github.com/JonasT/Blitwizard.git . || { echo "Failed to do git checkout."; exit 1; }
    rm -rf ./.git
    if [ "$2" != "android" ]; then
        sh autogen.sh || { echo "Autoconf generation failed."; exit 1; }
    fi

    # Remove things from source release we don't want
    rm .gitignore
    rm bin/.gitignore
    rm src/sdl/.gitignore
    rm src/vorbis/.gitignore
    rm src/ogg/.gitignore
    rm src/lua/.gitignore
    rm src/imgloader/png/.gitignore
    rm src/imgloader/zlib/.gitignore
    find ./ -iname "*.xcf" -delete
    cd ..

    # Rename to ./blitwizard/ and zip for source release
    rm -rf blitwizard/
    mv ./tarball ./blitwizard || { echo "Failed to rename tarball -> blitwizard."; exit 1; }
    if [ "$DOSOURCERELEASE" = "yes" ]; then
        mv ./blitwizard/ffmpeg-*.tar.bz2 ./
        zip -r -9 ./blitwizard-$RELEASEVERSION-source.zip ./blitwizard/ || { mv ./ffmpeg-*.tar.bz2 ./blitwizard/; echo "zip doesn't work as expected - is zip installed?"; exit 1; }
        mv ./ffmpeg-*.tar.bz2 ./blitwizard/
    fi
    cd blitwizard

    # Get dependencies
    rm deps.zip
    wget http://www.blitwizard.de/download-files/deps.zip || { echo "Failed to download deps.zip for dependencies"; exit 1; }
    unzip deps.zip

    # Get FFmpeg
    USE_FFMPEG_TAR="yes"
    wget http://www.blitwizard.de/ffmpeg.tar.bz2 || { USE_FFMPEG_TAR="no" }
    if [ "x$USE_FFMPEG_TAR" = xyes ]; then
        mkdir -p src/ffmpeg/
        cd src/ffmpeg/
        tar --strip-components=1 -xvjf ../../ffmpeg.tar.bz2
        cd ../../
    fi
else
    cd blitwizard
fi

# Do android build here
if [ "x$RELEASETYPE" = xandroid ]; then
    cd ..
    sh android/android.sh "$ANDROID_SDK_PATH" "$ANDROID_NDK_PATH" "$REDOWNLOAD" || { echo "Failed to complete Android build."; exit 1; }
    exit 0;
fi

# Do ./configure
if [ "$TARGETHOST" = "" ]; then
    CC="$CC" ./configure --disable-flac || { echo "./configure failed."; exit 1; }
else
    unset $CC
    ./configure --disable-flac --host="$TARGETHOST" || { echo "./configure failed"; exit 1; }
fi

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
cp ../blitwizard/ISSUES.txt ./
cp ../blitwizard/Credits.txt ./

if [ -e "./bin/blitwizard.exe" ]; then
    # Add easy start script
    cp ../blitwizard/Run-Blitwizard.bat ./
    
    # Add FFmpeg to windows distribution
#    cp ../../src/ffmpeg/libavformat/avformat.dll ./bin/avformat.dll
#    cp ../../src/ffmpeg/libavutil/avutil.dll ./bin/avutil.dll
#    cp ../../src/ffmpeg/libavcodec/avcodec.dll ./bin/avcodec.dll
#    cp ../blitwizard/ffmpeg-*.tar.bz2 ./
fi

# detect todos or unix2dos
UNIXTODOS="unix2dos"
$UNIXTODOS ./README*.txt || {
    UNIXTODOS="todos"
    $UNIXTODOS ./README*.txt || { echo "unix2dos/todos not working - is any of them installed?"; exit 1; }
}
$UNIXTODOS ./ISSUES.txt
$UNIXTODOS ./Credits.txt
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

