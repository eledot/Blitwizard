#!/bin/bash

cd ..

luatarget=`cat scripts/.buildinfo | grep luatarget | sed -e 's/^luatarget\=//'`
static_libs_use=`cat scripts/.buildinfo | grep STATIC_LIBS_USE | sed -e 's/^.*\=//'`
use_lib_flags=`cat scripts/.buildinfo | grep USE_LIB_FLAGS | sed -e 's/^.*\=//'`

changedhost=""

# Check if our previous build target was a different platform:
REBUILDALL="no"
if [ -f scripts/.depsarebuilt_luatarget ]; then
    depsluatarget=`cat scripts/.depsarebuilt_luatarget`
    if [ "$luatarget" != "$depsluatarget" ]; then
        # target has changed
        REBUILDALL="yes"
    fi
else
    # we had no previous known target at all, so obviously yes!
    REBUILDALL="yes"
fi

# Check if our previous build flags were different:
REBUILDCORE="no"
if [ -f scripts/.depsarebuilt_flags ]; then
    depsuselibflags=`cat scripts/.depsarebuilt_flags`
    if [ "$use_lib_flags" != "$depsuselibflags" ]; then
        # flags have changed
        REBUILDCORE="yes"
    fi
else
    # we had no previous known flags at all, so obviously yes!
    REBUILDCORE="yes"
fi

if [ "$REBUILDALL" = "yes" ]; then
    REBUILDCORE="yes"
    echo "Deps will be rebuilt to match new different target.";
    changedhost="yes";
    rm libs/libblitwizard*.a > /dev/null 2>&1
    rm libs/libimglib.a > /dev/null 2>&1
fi
if [ "$REBUILDCORE" = "yes" ]; then
    echo "Core will be rebuilt to match different build flags or target.";
    rm src/*.o > /dev/null 2>&1
fi

CC=`cat scripts/.buildinfo | grep CC | sed -e 's/^.*\=//'`
RANLIB=`cat scripts/.buildinfo | grep RANLIB | sed -e 's/^.*\=//'`
AR=`cat scripts/.buildinfo | grep AR | sed -e 's/^.*\=//'`
HOST=`cat scripts/.buildinfo | grep HOST | sed -e 's/^.*\=//'`
MACBUILD=`cat scripts/.buildinfo | grep MACBUILD | sed -e 's/^.*\=//'`

if [ "$MACBUILD" = "yes" ]; then
    # Enforce darwin gcc since llvm-gcc hates libvorbis
    CC="clang"
fi

export CC="$CC"
export AR="$AR"

dir=`pwd`

echo "Will build static dependencies now."

if [ ! -e libs/libimglib.a ]; then
    if [ -n "`echo $static_libs_use | grep imgloader`" ]; then
        # static png:
        if [ -n "`echo $static_libs_use | grep png`" ]; then
            echo "Compiling libpng..."
            cd src/imgloader && make deps-png || { echo "Failed to compile libpng"; exit 1; }
            cd $dir
        fi
        #static zlib
        if [ -n "`echo $static_libs_use | grep zlib`" ]; then
            echo "Compiling zlib..."
            cd src/imgloader && make deps-zlib || { echo "Failed to compile zlib"; exit 1; }
            cd $dir
        fi
        echo "Compiling imgloader..."
        # Our custom threaded image loader wrapper
        cd src/imgloader && make || { echo "Failed to compile imgloader"; exit 1; }
    fi
    cd $dir
fi

if [ ! -e libs/libblitwizardogg.a ]; then
    if [ -n "`echo $static_libs_use | grep ogg`" ]; then
        echo "Compiling libogg..."
        # Build ogg
        cd src/ogg && ./configure --host="$HOST" --disable-shared --enable-static && make clean && make || { echo "Failed to compile libogg"; exit 1; }
        cd $dir
    fi
fi

if [ ! -e libs/libblitwizardFLAC.a ]; then
    if [ -n "`echo $static_libs_use | grep FLAC`" ]; then
        echo "Compiling libFLAC..."
        if [ -n "`echo $static_libs_use | grep ogg`" ]; then
            # Build flac and tell it where ogg is
            oggincludedir="`pwd`/src/ogg/include/"
            ogglibrarydir="`pwd`/src/ogg/src/.libs/"
            cd src/flac && ./configure --host="$HOST" --with-ogg-libraries="$ogglibrarydir" --with-ogg-includes="$oggincludedir" --enable-static --disable-shared --disable-thorough-tests --disable-xmms-plugin --disable-cpplibs --disable-doxygen-docs && make clean && make || { echo "Failed to compile libFLAC"; exit 1; }
        else
            # Build flac and make it guess where ogg is
            cd src/flac && ./configure --host="$HOST" --enable-static --disable-shared --disable-thorough-tests --disable-xmms-plugin --disable-cpplibs --disable-doxygen-docs && make clean && make || { echo "Failed to compile libFLAC"; exit 1; }
        fi
        cd $dir
    fi
fi

if [ ! -e libs/libblitwizardspeex.a ]; then
    if [ -n "`echo $static_libs_use | grep speex`" ]; then
        echo "Compiling libspeex..."
        if [ -n "`echo $static_libs_use | grep ogg`" ]; then
            # Build speex and remember to tell it where ogg is
            oggincludedir="`pwd`/src/ogg/include/"
            ogglibrarydir="`pwd`/src/ogg/src/.libs/"
            cd src/speex && ./configure --host="$HOST" --with-ogg-libraries="$ogglibrarydir" --with-ogg-includes="$oggincludedir" --disable-oggtest --disable-shared --enable-static && make clean && make || { echo "Failed to compile libvorbis"; exit 1; }
        else
            # Build speex
            cd src/speex && ./configure --host="$HOST" --disable-oggtest --disable-shared --enable-static && make clean && make || { echo "Failed to compile libvorbis"; exit 1; }
        fi
        cd $dir
    fi
fi

NOVORBIS="no"
if [ ! -e libs/libblitwizardvorbis.a ]; then
    NOVORBIS="yes"
fi
if [ ! -e libs/libblitwizardvorbisfile.a ]; then
    NOVORBIS="yes"
fi

if [ "$NOVORBIS" = "yes" ]; then
    if [ -n "`echo $static_libs_use | grep vorbis`" ]; then
        echo "Compiling libvorbis..."
        if [ -n "`echo $static_libs_use | grep ogg`" ]; then
            # Build vorbis and remember to tell it where ogg is
            oggincludedir="`pwd`/src/ogg/include/"
            ogglibrarydir="`pwd`/src/ogg/src/.libs/"
            if [ "$MACBUILD" != "yes" ]; then
                cd src/vorbis && ./configure --host="$HOST" --with-ogg-libraries="$ogglibrarydir" --with-ogg-includes="$oggincludedir" --disable-oggtest --disable-docs --disable-examples --disable-shared --enable-static && make clean && make || { echo "Failed to compile libvorbis"; exit 1; }
            else
                cd src/vorbis && ./configure --with-ogg-libraries="$ogglibrarydir" --with-ogg-includes="$oggincludedir" --disable-oggtest --disable-docs --disable-examples --disable-oggtest --disable-shared --enable-static && make clean && make || { echo "Failed to compile libvorbis"; exit 1; }
            fi
        else
            # Build vorbis and let's hope it finds the system ogg
            if [ "$MACBUILD" != "yes" ]; then
                cd src/vorbis && ./configure --host="$HOST" --disable-oggtest --disable-docs --disable-examples --disable-shared --enable-static && make clean && make || { echo "Failed to compile libvorbis"; exit 1; }
            else
                cd src/vorbis && ./configure --disable-oggtest --disable-docs --disable-examples --disable-oggtest --disable-shared --enable-static && make clean && make || { echo "Failed to compile libvorbis"; exit 1; }
            fi
        fi
        cd $dir
    fi
fi

if [ ! -e libs/libblitwizardbox2d.a ]; then
    if [ -n "`echo $static_libs_use | grep box2d`" ]; then
        # Build box2d
        box2dpremake="no"
        cd $dir
        export CMAKE_SYSTEM_NAME=""
        CROSSCMAKEFLAG=""
        if [ "$luatarget" = "mingw" ]; then
            sed "s/AUTOTOOLS_HOST/${HOST}/g" scripts/box2dforwindows.cmake.template > scripts/box2dforwindows.cmake
            CROSSCMAKEFLAG="-DCMAKE_TOOLCHAIN_FILE=../../scripts/box2dforwindows.cmake -Dhost_platform=${HOST} "
        fi
        cd src/box2d/
        echo "Appended option: (${CROSSCMAKEFLAG})"
        cmake ${CROSSCMAKEFLAG} -DBOX2D_INSTALL=OFF -DBOX2D_BUILD_SHARED=OFF -DBOX2D_BUILD_STATIC=ON -DBOX2D_BUILD_EXAMPLES=OFF -DCMAKE_BUILD_TYPE=Release . || {
            cd $dir
            cd src/box2d && premake4 gmake || { echo "Failed to configure box2d"; exit 1; }
            box2dpremake="yes"
        }
        cd $dir
        if [ "$box2dpremake" = "yes" ]; then
            echo "Doing Box2D build (premake configured)"
            cd src/box2d && make config="release" || { echo "Failed to build box2d"; exit 1; }
        else
            echo "Doing Box2D build (cmake configured)"
            cd src/box2d && make || { echo "Failed to build box2d"; exit 1; }
        fi
        cd $dir
    fi
fi

if [ ! -e libs/libblitwizardSDL.a ]; then
    if [ -n "`echo $static_libs_use | grep SDL2`" ]; then
        # Build SDL 2
        if [ "$MACBUILD" != "yes" ]; then
            rm -r src/sdl/.hg/
            cd src/sdl && ./configure --host="$HOST" --enable-assertions=release --enable-ssemath --disable-pulseaudio --enable-sse2 --disable-shared --enable-static || { echo "Failed to compile SDL2"; exit 1; }
        else
            rm -r src/sdl/.hg/
            cd src/sdl && ./configure --enable-assertions=release --enable-ssemath --disable-pulseaudio --enable-sse2 --disable-shared --enable-static || { echo "Failed to compile SDL2"; exit 1; }
        fi
        cd $dir
        if [ "$changeddeps" = "yes" ]; then
            cd src/sdl && make clean || { echo "Failed to compile SDL2"; exit 1; }
            cd $dir
        fi
        cd src/sdl && make || { echo "Failed to compile SDL2"; exit 1; }
        cd $dir
    fi
fi

if [ ! -e libs/libblitwizardlua.a ]; then
    if [ -n "`echo $static_libs_use | grep lua`" ]; then
        # Avoid the overly stupid Lua build script which doesn't even adhere to $CC
        cd src/lua/ && rm -f src/liblua.a && rm -rf build/ || { echo "Failed to compile Lua 5"; exit 1; }
        cd $dir
        cd src/lua/ && mkdir -p build/ && cp src/*.c build/ && cp src/*.h build/ || { echo "Failed to compile Lua 5"; exit 1; }
        cd $dir
        if [ -z "$AR" ]; then
            AR="ar"
        fi
        cd src/lua/build && rm lua.c && rm luac.c && $CC -c -O2 *.c && $AR rcs ../src/liblua.a *.o || { echo "Failed to compile Lua 5"; exit 1; }
        cd $dir
    fi
fi

# Wipe out the object files of blitwizard if we need to
if [ "$changedhost" = "yes" ]; then
    rm -f src/*.o
fi

# Copy libraries
if [ ! -e libs/libblitwizardSDL.a ]; then
    if [ -n "`echo $static_libs_use | grep SDL2`" ]; then
        cp src/sdl/build/.libs/libSDL2.a libs/libblitwizardSDL.a || { echo "Failed to copy SDL2 library"; exit 1; }
    fi
fi
if [ "$NOVORBIS" = "yes" ]; then
    if [ -n "`echo $static_libs_use | grep vorbis`" ]; then
        cp src/vorbis/lib/.libs/libvorbis.a libs/libblitwizardvorbis.a || { echo "Failed to copy libvorbis"; exit 1; }
        cp src/vorbis/lib/.libs/libvorbisfile.a libs/libblitwizardvorbisfile.a || { echo "Failed to copy libvorbisfile"; exit 1; }
    fi
fi
if [ ! -e libs/libblitwizardogg.a ]; then
    if [ -n "`echo $static_libs_use | grep ogg`" ]; then
        cp src/ogg/src/.libs/libogg.a libs/libblitwizardogg.a || { echo "Failed to copy libogg"; exit 1; }
    fi
fi
if [ ! -e libs/libblitwizardFLAC.a ]; then
    if [ -n "`echo $static_libs_use | grep FLAC`" ]; then
        cp src/flac/src/libFLAC/.libs/libFLAC.a libs/libblitwizardFLAC.a || { echo "Failed to copy libFLAC"; exit 1; }
    fi
fi
if [ ! -e libs/libblitwizardspeex.a ]; then
    if [ -n "`echo $static_libs_use | grep speex`" ]; then
        cp src/speex/libspeex/.libs/libspeex.a libs/libblitwizardspeex.a || { echo "Failed to copy libspeex"; exit 1; }
    fi  
fi
if [ ! -e libs/libblitwizardspeexdsp.a ]; then
    if [ -n "`echo $static_libs_use | grep speex`" ]; then
        cp src/speex/libspeex/.libs/libspeexdsp.a libs/libblitwizardspeexdsp.a || { echo "Failed to copy libspeexdsp"; exit 1; }
    fi  
fi
if [ ! -e libs/libimglib.a ]; then
    if [ -n "`echo $static_libs_use | grep imgloader`" ]; then
        cp src/imgloader/libimglib.a libs/ || { echo "Failed to copy imglib"; exit 1; }
    fi
fi
if [ ! -e libs/libblitwizardpng.a ]; then
    if [ -n "`echo $static_libs_use | grep png`" ]; then
        cp src/imgloader/libcustompng.a libs/libblitwizardpng.a
    fi
fi
if [ ! -e libs/libblitwizardzlib.a ]; then
    if [ -n "`echo $static_libs_use | grep zlib`" ]; then
        cp src/imgloader/libcustomzlib.a libs/libblitwizardzlib.a
    fi
fi
if [ ! -e libs/libblitwizardlua.a ]; then
    cp src/lua/src/liblua.a libs/libblitwizardlua.a
fi
if [ ! -e libs/libblitwizardbox2d.a ]; then
    if [ -n "`echo $static_libs_use | grep box2d`" ]; then
        BOX2DCOPIED1="true"
        BOX2DCOPIED2="true"
        cp src/box2d/Box2D/libBox2D.a libs/libblitwizardbox2d.a || { BOX2DCOPIED1="false"; }
        cp src/box2d/Box2D/Box2D.lib libs/libblitwizardbox2d.a || { BOX2DCOPIED2="false"; }
        if [ "$BOX2DCOPIED1" = "false" ]; then
            if [ "$BOX2DCOPIED2" = "false" ]; then
                echo "Failed to copy Box2D library"
                exit 1
            fi
        fi
    fi
fi

# Remember for which target we built
echo "$luatarget" > scripts/.depsarebuilt_luatarget
echo "$use_lib_flags" > scripts/.depsarebuilt_flags

