#!/bin/bash

# Android build script. Do not call directly, use create-release-archive.sh instead!

# Deal with overly curious users first:
if [ -z "$1" ]; then
    echo "Please don't run this directly. Run create-release-archive.sh instead."
    exit 1;
fi
if [ "$1" = "-h" ]; then
    echo "Please don't run this directly. Run create-release-archive.sh instead."
    exit 1;
fi
if [ "$1" = "-help" ]; then
    echo "Please don't run this directly. Run create-release-archive.sh instead."
    exit 1;
fi
if [ "$1" = "--help" ]; then
    echo "Please don't run this directly. Run create-release-archive.sh instead."
    exit 1;
fi

ANDROID_SDK_PATH="$1"
ANDROID_NDK_PATH="$2"
REDOWNLOADED="$3"
ANDROIDINTVERSION=`cat ../configure.ac | grep "Int version" | sed "s/# Int version: //g" | sed "s/ (.*//g"`

COMPILE="yes"

if [ "$REDOWNLOADED" = "yes" ]; then
    # Copy the project and SDL
    rm -rf ./blitwizard-android/
    cp -R blitwizard/src/sdl/android-project ./blitwizard-android/ || { echo "Failed to copy android-project"; exit 1; }
    cp -R blitwizard/src/sdl/ ./blitwizard-android/jni/SDL/ || { echo "Failed to copy SDL"; exit 1; }

    # Copy templates
    mkdir -p blitwizard-android/assets/
    cp -R blitwizard/templates/ ./blitwizard-android/assets/templates/

    # Generate templates init list
    sh android/generate-template-script.sh > blitwizard-android/assets/templates/filelist.lua

    # Ensure our SDL project has the STL for Box2D
    echo "APP_STL := stlport_static" | cat - blitwizard-android/jni/Android.mk > /tmp/androidmk && mv /tmp/androidmk blitwizard-android/jni/Android.mk
    echo "STLPORT_FORCE_REBUILD := true" | cat - blitwizard-android/jni/Android.mk > /tmp/androidmk && mv /tmp/androidmk blitwizard-android/jni/Android.mk

    # Tell SDL to build statically
    cat blitwizard-android/jni/SDL/Android.mk | sed "s/include \\\$(BUILD_SHARED_LIBRARY)/include \$(BUILD_STATIC_LIBRARY)/g" > blitwizard-android/jni/SDL/Android.mk.2
    mv blitwizard-android/jni/SDL/Android.mk.2 blitwizard-android/jni/SDL/Android.mk

    # SDL fix for static Android build (gives an error otherwise)
    cat blitwizard-android/jni/SDL/src/main/android/SDL_android_main.cpp | sed "s/JNI_OnLoad/JNI_OnLoadUnused/g" > blitwizard-android/jni/SDL/src/main/android/SDL_android_main.cpp.2
    mv blitwizard-android/jni/SDL/src/main/android/SDL_android_main.cpp.2 blitwizard-android/jni/SDL/src/main/android/SDL_android_main.cpp

    # Another SDL fix for static build: don't load SDL at runtime
    cat blitwizard-android/src/org/libsdl/app/SDLActivity.java | sed "s/System.loadLibrary(\"SDL2\");/\/\/System.loadLibrary(\"SDL2\")/g" > blitwizard-android/src/org/libsdl/app/SDLActivity.java.2
    mv blitwizard-android/src/org/libsdl/app/SDLActivity.java.2 blitwizard-android/src/org/libsdl/app/SDLActivity.java

    # Make sure the status bar is gone in fullscreen
    cat blitwizard-android/AndroidManifest.xml | sed "s/<application android:label=\"@string\\/app_name\" android:icon=\"@drawable\\/icon\">/<uses-sdk android:minSdkVersion=\"4\"\\/> <application android:label=\"@string\\/app_name\" android:icon=\"@drawable\\/icon\" android:theme=\"@android:style\\/Theme.NoTitleBar.Fullscreen\">/g" > blitwizard-android/AndroidManifest.xml.2
    mv blitwizard-android/AndroidManifest.xml.2 blitwizard-android/AndroidManifest.xml

    # Enforce landscape orientation since that is what all fancy games use
    cat blitwizard-android/AndroidManifest.xml | sed "s/<activity android:name=\"SDLActivity\"/<activity android:name=\"SDLActivity\" android:configChanges=\"orientation\" android:screenOrientation=\"landscape\"/g" > blitwizard-android/AndroidManifest.xml.2
    mv blitwizard-android/AndroidManifest.xml.2 blitwizard-android/AndroidManifest.xml

    # Preserve original manifest
    cp blitwizard-android/AndroidManifest.xml blitwizard-android/AndroidManifest.xml.orig
else
    if [ -f "blitwizard-android/libs/armeabi/libmain.so" ]; then
        read -p "Recompile NDK code? [y/N]"
        COMPILE="no"
        if [ "$REPLY" = "y" ]; then
            COMPILE="yes"
        fi
        if [ "$REPLY" = "Y" ]; then
            COMPILE="yes"
        fi
    fi
fi

# Copy android config
cp blitwizard-android/jni/SDL/include/SDL_config_android.h blitwizard-android/jni/SDL/include/SDL_config.h

# Copy original manifest
cp blitwizard-android/AndroidManifest.xml.orig blitwizard-android/AndroidManifest.xml

# Ask the user for application version
echo "Please enter a program version number, like 1 or 2."
read -p "The number should be incremented for each new release:"
PROGVERSION="$REPLY"
echo "Supplied version number is: [$PROGVERSION]"
test -n "$input" -o -n "`echo $PROGVERSION | tr -d '[0-9]'`" && { echo "You did not input a simple number. Please use only 0-9."; exit 1; }
echo "Please enter a program version string, like 1.0."
read -p "This version string will be visible to the user:"
PROGVERSIONFULL="$REPLY"
if [ -z "$PROGVERSIONFULL" ]; then
    echo "You did not enter a program veersion.";
    exit 1;
fi

# Insert version
cat blitwizard-android/AndroidManifest.xml | sed "s/android:versionCode=\"1\"/android:versionCode=\"${PROGVERSION}\"/g" | sed "s/android:versionName=\"1\\.0\"/android:versionName=\"${PROGVERSIONFULL}\"/g" > blitwizard-android/AndroidManifest.xml.2
mv blitwizard-android/AndroidManifest.xml.2 blitwizard-android/AndroidManifest.xml

# Ask the user some things:
echo "Type intended app name (or nothing) and then press [ENTER]:"
read app_name
if [ -z "$app_name" ]; then
    app_name="Blitwizard App"
fi
echo "App name will be $app_name"
echo "You can include a blitwizard game which blitwizard will run."
echo "For this, you can provide the path of a directory containing it."
echo "Please note !!ONLY THE GAME FILES!! should be in that directory,"
echo "not any other files like blitwizard binaries, examples/templates"
echo "folder or anything."
echo "Type a folder path or nothing and press [ENTER]:"
read game_files_path

if [ -n "$game_files_path" ]; then
    if [ ! -e "$game_files_path"/game.lua ]; then
        echo "game.lua is not present. Please check your game folder path!"
        exit 1;
    fi
    if [ -e "$game_files_path"/templates/ ]; then
        echo "Folder contains \"templates\" sub folder or file. Please use a directory with only your game files in it!"
        exit 1
    fi
    echo "Copying game files..."
    cp -R "$game_files_path"/* blitwizard-android/assets/
    echo "Game files copied."
else
    echo "No game file path given, not integrating any resources."
fi

# Get the blitwizard version
blitwizard_version=`grep AC_INIT blitwizard/configure.ac | sed -e "s/AC_INIT[(][[]blitwizard[]], [[]//g" | sed -e "s/[]])//g"`

if [ "$COMPILE" = "yes" ]; then
    if [ ! -d "blitwizard-android/src/vorbis" ]; then
        # Prepare vorbis
        cp -R blitwizard/src/vorbis/ ./blitwizard-android/jni/vorbis/
        cp android/Android-vorbis.mk ./blitwizard-android/jni/vorbis/Android.mk
        rm blitwizard-android/jni/vorbis/lib/psytune.c # dead code (see comments inside)
        rm blitwizard-android/jni/vorbis/lib/tone.c # program with main()
        rm blitwizard-android/jni/vorbis/lib/barkmel.c # program with main()

        # Vorbis define for ogg_types.h hack
        cat blitwizard-android/jni/vorbis/lib/os.h | sed "s/\#define _OS_H/#define _OS_H\n#define VORBIS_HACK/g" > blitwizard-android/jni/vorbis/lib/os.h.2
    fi

    # Prepare ogg
    if [ ! -d "blitwizard-android/src/ogg" ]; then
        cp -R blitwizard/src/ogg/ ./blitwizard-android/jni/ogg/
        cp android/Android-ogg.mk ./blitwizard-android/jni/ogg/Android.mk
        cp android/ogg_config_types.h blitwizard-android/jni/ogg/include/ogg/config_types.h
    fi

    # Prepare Box2D
    if [ ! -d "blitwizard-android/src/box2d" ]; then
        cp -R blitwizard/src/box2d/ ./blitwizard-android/jni/box2d/
        cp android/Android-box2d.mk ./blitwizard-android/jni/box2d/Android.mk
    fi

    # Prepare imgloader
    if [ ! -d "blitwizard-android/jni/imgloader" ]; then
        mkdir blitwizard-android/jni/imgloader/
        cp blitwizard/src/imgloader/*.c blitwizard-android/jni/imgloader/
        cp blitwizard/src/imgloader/*.h blitwizard-android/jni/imgloader/
        cp android/Android-imgloader.mk ./blitwizard-android/jni/imgloader/Android.mk
    fi

    # Prepare png
    if [ ! -d "blitwizard-android/src/png" ]; then
        cp -R blitwizard/src/imgloader/png/ ./blitwizard-android/jni/png/
        rm blitwizard-android/jni/png/pngtest.c # program with main()
        rm blitwizard-android/jni/png/pngvalid.c # we do not need this
        cp android/Android-png.mk ./blitwizard-android/jni/png/Android.mk
        cp blitwizard-android/jni/png/scripts/pnglibconf.h.prebuilt blitwizard-android/jni/png/pnglibconf.h
    fi

    # Prepare zlib
    if [ ! -d "blitwizard-android/src/zlib" ]; then
        cp -R blitwizard/src/imgloader/zlib/ ./blitwizard-android/jni/zlib/
        rm blitwizard-android/jni/zlib/example.c # program with main()
        cp android/Android-zlib.mk ./blitwizard-android/jni/zlib/Android.mk
    fi

    # Prepare Lua
    if [ ! -d "blitwizard-android/jni/lua" ]; then
        mkdir blitwizard-android/jni/lua/
        cp blitwizard/src/lua/src/*.c blitwizard-android/jni/lua/
        cp blitwizard/src/lua/src/*.h blitwizard-android/jni/lua/
        rm blitwizard-android/jni/lua/lua.c
        rm blitwizard-android/jni/lua/luac.c
        cp android/Android-lua.mk blitwizard-android/jni/lua/Android.mk
        cat blitwizard-android/jni/lua/llex.c | sed -e "s/#define llex_c/#define llex_c\nchar decpointstr() {return '.';}\n#define getlocaledecpoint (decpointstr)/g" > blitwizard-android/jni/lua/llex2.c
        mv blitwizard-android/jni/lua/llex2.c blitwizard-android/jni/lua/llex.c
    fi

    # Blitwizard Android.mk:
    source_file_list="`cat ../src/Makefile.am | grep blitwizard_SOURCES | sed -e 's/^blitwizard_SOURCES \= //'`"
    if [ ! -f "blitwizard-android/jni/src/main.c" ]; then
        cp blitwizard/src/*.c blitwizard-android/jni/src
        cp blitwizard/src/*.cpp blitwizard-android/jni/src
        cp blitwizard/src/*.h blitwizard-android/jni/src
        cp android/Android-blitwizard.mk blitwizard-android/jni/src/Android.mk
        cat blitwizard-android/jni/src/Android.mk | sed -e "s/SOURCEFILELIST/${source_file_list}/g" > blitwizard-android/jni/src/Android2.mk
        cat blitwizard-android/jni/src/Android2.mk | sed "s/VERSIONINSERT/\\\\\"${blitwizard_version}\\\\\"/g" > blitwizard-android/jni/src/Android.mk
    fi
fi

# Use the Android NDK/SDK to complete our project:
cd blitwizard-android
export HOST_AWK="awk"

# Install our Application.mk
cp ../android/Application.mk ./jni/Application.mk

# NDK build:
if [ "$COMPILE" = "yes" ]; then
    mv "$ANDROID_NDK_PATH/prebuilt/linux-x86/bin/awk" "$ANDROID_NDK_PATH/prebuilt/linux-x86/bin/awk_"
    "$ANDROID_NDK_PATH/ndk-build" || { echo "NDK build failed."; cd ..; exit 1; }
fi

# Regenerate build.xml (SDL build.xml is outdated) and prepare some strings:
rm -f build.xml
"$ANDROID_SDK_PATH/tools/android" update project -p . --target android-10 || { echo "android update failed."; cd ..; exit 1; }
cat build.xml | sed -e "s/SDLActivity/${app_name}/g" > build2.xml
mv build2.xml build.xml
cat res/values/strings.xml | sed -e "s/SDL App/${app_name}/g" > res/values/strings2.xml
mv res/values/strings2.xml res/values/strings.xml

# Do final SDK build
echo "sdk.dir=$ANDROID_SDK_PATH" > local.properties
echo "Removing old .apk and .class files..."
rm bin/classes/org/libsdl/app/*.class
rm bin/*.apk
echo "Running ant"
ant debug || { echo "ant failed."; cd ..; exit 1; }
cd ..

echo "Copying new .apk..."
rm "${app_name}.apk"
cp "blitwizard-android/bin/${app_name}-debug.apk" "${app_name}.apk"
echo "Done,"

# Success!
echo ""
echo ""
echo " -- Success:"
echo "Android \"${app_name}.apk\" should be complete."
echo "You can now ship your blitwizard game to the phones!"
echo "Small side note:"
echo "  You should include the contents of README-libs.txt"
echo "  in an about screen inside your app."
echo "  Otherwise some people (third-party library authors)"
echo "  might get unhappy."
exit 0

