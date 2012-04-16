
# blitwizard configure.ac - autoconf generated, extended by Jonas Thiem
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])

AC_INIT([blitwizard], [0.2.1])

# Checks for programs.
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
AC_CANONICAL_HOST

rm scripts/.depsarebuilt

#AC_DISABLE_SHARED
#AC_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])

# Enable/disable features:

use_sdlgraphics=yes
AC_ARG_WITH([sdl-graphics],
    [AS_HELP_STRING([--with-sdl-graphics],[use graphics support through SDL (default is yes)])],
[use_sdlgraphics=$withval], [use_sdlgraphics=no])

use_sdlaudio=yes
AC_ARG_WITH([sdl-audio],
    [AS_HELP_STRING([--with-sdl-audio],[use audio support through SDL (default is yes)])],
[use_sdlaudio=$withval], [use_sdlaudio=no])


# Evaluate target os:

maybeos=`echo $EXEEXT | grep exe`


# Remember which libs we want to build ourselves for static use:

STATIC_LIBS_USE=""


# Remember which flags we need because we don't use some libs at all

GRAPHICS_FEATURE_ENABLED="yes"
NULL_GRAPHICS_FEATURE_ENABLED="yes"
AUDIO_FEATURE_ENABLED="yes"
NULL_AUDIO_FEATURE_ENABLED="yes"
FLAC_FEATURE_ENABLED="yes"
SPEEX_RESAMPLING_FEATURE_ENABLED="yes"
FFMPEG_FEATURE_ENABLED="yes"
PHYSICS_FEATURE_ENABLED="yes"


# Lib deps info text for later output:

LIBS_DEP_INFO_TEXT="
*** Dependency scan results ***"


# Check whether ffmpeg is available

AS_IF([test -e "src/ffmpeg/libavcodec/avcodec.h"],[
    AC_MSG_NOTICE([FFmpeg source code appears to be present, will compile with FFmpeg support.])
    AS_IF([test ! -e "src/ffmpeg/libavutil/avconfig.h"], [
        AC_WARN([FFmpeg is not configured yet. FFmpeg will be now configured (but not built) with recommended options. If you want to do that yourself, please cancel and do it now, then rerun blitwizard ./configure.])
        { cd src/ffmpeg && ./configure --disable-libgsm --enable-runtime-cpudetect  --enable-version3 --disable-gpl --enable-shared --disable-doc --disable-ffmpeg --disable-avconv --disable-ffplay --disable-ffprobe --disable-ffserver --disable-protocols --disable-devices; } || { AC_MSG_ERROR([Failed to configure FFmpeg.]); exit 1; }
        cd ../../
    ])
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  FFmpeg headers from tarball: yes"
],[
    AC_WARN([FFmpeg source code is not present or not properly placed. Blitwizard will NOT have any FFmpeg support.])
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  FFmpeg headers from tarball: no"
    FFMPEG_FEATURE_ENABLED="no"
])


# Check for SDL2 availability:

AC_MSG_CHECKING([for static SDL2 library])
AS_IF([test ! -f "src/sdl/src/SDL.c"],[
    AC_MSG_RESULT([no])
    AC_CHECK_LIB([SDL2], [SDL_Init],[
        #shared lib is there!
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  SDL2: shared"
    ],[
        #nothing there
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  SDL2: no"
        GRAPHICS_FEATURE_ENABLED="no, need SDL2"
        AUDIO_FEATURE_ENABLED="no, need SDL2"
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} SDL2"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  SDL2: static"
])


# Check for libpng availability:

AC_MSG_CHECKING([for static png library])
AS_IF([test ! -f "src/imgloader/png/png.c"],[
    AC_MSG_RESULT([no])
    AC_CHECK_LIB([png], [png_create_read_struct],[
        #shared lib is there!
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libpng: shared"
    ],[
        #nothing there
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libpng: no"
        GRAPHICS_FEATURE_ENABLED="no, need libpng"
        NULL_GRAPHICS_FEATURE_ENABLED="no, need libpng"
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} png"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libpng: static"
])


# Check for zlib availability:

AC_MSG_CHECKING([for static zlib library])
AS_IF([test ! -f "src/imgloader/zlib/gzlib.c"],[
    AC_MSG_RESULT([no])
    AC_CHECK_LIB([z], [inflateInit2],[
        #shared lib is there!
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  zlib: shared"
    ],[
        #nothing there
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  zlib: no"
        GRAPHICS_FEATURE_ENABLED="no, need libpng"
        NULL_GRAPHICS_FEATURE_ENABLED="no, need libpng"
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} zlib"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  zlib: static"
])


# Check for libvorbis availability:

AC_MSG_CHECKING([for static vorbis/vorbisfile library])
AS_IF([test ! -f "src/vorbis/lib/vorbisenc.c"],[
    AC_MSG_RESULT([no])
    AC_CHECK_LIB([vorbisfile], [ov_info],[
        AC_CHECK_LIB([vorbis], [vorbis_block_init],[
            #shared lib is there!
            LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libvorbis/vorbisfile: shared"
        ],[
            #incomplete
            LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libvorbis/vorbisfile: no"
            AUDIO_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
            NULL_AUDIO_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
        ])
    ],[
        #nothing there
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libvorbis/vorbisfile: no"
        AUDIO_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
        NULL_AUDIO_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} vorbis"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libvorbis/vorbisfile: static"
])


# Check for libogg availability:

AC_MSG_CHECKING([for static ogg library])
AS_IF([test ! -f "src/ogg/src/framing.c"],[
    AC_MSG_RESULT([no])
    AC_CHECK_LIB([ogg], [oggpack_read],[
        #shared lib is there!
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libogg: shared"
    ],[
        #nothing there
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libogg: no"
        AUDIO_FEATURE_ENABLED="no, need libogg"
        NULL_AUDIO_FEATURE_ENABLED="no, need libogg"
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} ogg"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libogg: static"
])


# Check for liblua availability:

AC_MSG_CHECKING([for static lua library])
AS_IF([test ! -f "src/lua/src/lua.h"],[
    AC_MSG_RESULT([no])
    AC_CHECK_LIB([lua], [lua_close],[
        #shared lib is there!
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  liblua: shared"
    ],[
        #nothing there
        AC_MSG_ERROR([Lua is required for blitwizard.])
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} lua"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  liblua: static"
])


# Check for libbox2d availability:

AC_MSG_CHECKING([for static Box2D library])
AS_IF([test ! -f "src/box2d/Box2D/Box2D.h"],[
    AC_MSG_RESULT([no])
    AC_CHECK_LIB([box2d], [b2Alloc],[
        #shared lib is there!
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libbox2d: shared"
    ],[
        #nothing there
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libbox2d: no"
        PHYSICS_FEATURE_ENABLED="no, need libbox2d"
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} box2d"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libbox2d: static"
])


# Check for libFLAC availability:

AC_MSG_CHECKING([for static libFLAC library])
AS_IF([test ! -f "src/flac/src/libFLAC/lpc.c"],[
    AC_MSG_RESULT([no])
    AC_CHECK_LIB([FLAC], [FLAC__stream_decoder_new],[
        #shared lib is there!
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libFLAC: shared"
    ],[
        #nothing there
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libFLAC: no"
        FLAC_FEATURE_ENABLED="no, need libFLAC"
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} FLAC"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libFLAC: static"
])


# Check for libspeex availability:

AC_MSG_CHECKING([for static libspeex library])
AS_IF([test ! -f "src/speex/src/speexdec.c"],[
    AC_MSG_RESULT([no])
    AC_CHECK_LIB([speexdsp], [speex_resampler_init],[
        AC_CHECK_LIB([speex], [speex_encoder_init],[
            #shared lib is there!
            LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libspeex: shared"
        ],[
            #nothing here
            LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libspeex: no"
            SPEEX_RESAMPLING_FEATURE_ENABLED="no, need libspeex"
        ])
    ],[
        #incomplete
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libspeex: no"
        SPEEX_RESAMPLING_FEATURE_ENABLED="no, need libspeex"
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} speex"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libspeex: static"
])


# Os dependent stuff:

AS_IF([test -z "$maybeos"],[
    AC_MSG_NOTICE([Trying to compile for a Unix target.]) 
    # >> Unix

    MACTEST=`uname -a | grep Darwin`
    AS_IF([test -z "$MACTEST"],[
        LIBS="-pthread -ldl -lm"
        WRITELINE="\nluatarget=posix\nCC=$CC\nAR=$AR\nSTATIC_LIBS_USE=$STATIC_LIBS_USE=\nHOST=$TGHOST\nRANLIB=$RANLIB\nMACBUILD=no"
    ],[
        LIBS="$LIBS -lm -liconv -pthread -framework CoreServices -framework Carbon -framework AudioUnit -framework ForceFeedback -framework OpenGL -framework AppKit -framework Cocoa -framework Foundation -framework IOKit -framework CoreAudio"
        WRITELINE="\nEXEEXT=$EXEEXT\nluatarget=posix\nCC=$CC\nSTATIC_LIBS_USE=$STATIC_LIBS_USE\nAR=$AR\nHOST=$TGHOST\nRANLIB=$RANLIB\nMACBUILD=yes"
    ])

    echo -e "$WRITELINE" > scripts/.buildinfo
    CFLAGS="$CFLAGS $USE_FFMPEG `cd scripts && sh ./checksse.sh`"   

    AC_CHECK_LIB([dl], [dlopen],, AC_MSG_ERROR(Cannot find all the necessary functions. Try to install the necessary libraries and headers))

    # Checks for header files.
    AC_CHECK_HEADERS([arpa/inet.h dlfcn.h ctype.h fcntl.h netdb.h netinet/in.h stdarg.h sys/stat.h sys/socket.h sys/wait.h],,AC_MSG_ERROR(Cannot find all the necessary headers. Try to install the necessary libraries and headers))
    AC_CHECK_HEADERS([malloc.h])


    # Checks for typedefs, structures, and compiler characteristics.
    #AC_HEADER_STDBOOL

    AC_FUNC_ALLOCA(,,AC_MSG_ERROR(Cannot find all the necessary functions. Try to install the necessary libraries and headers))
    #AC_FUNC_CHOWN(,,AC_MSG_ERROR(Cannot find all the necessary functions. Try to install the necessary libraries and headers))
    #AC_FUNC_FORK(,,AC_MSG_ERROR(Cannot find all the necessary functions. Try to install the necessary libraries and headers))
    AC_CHECK_FUNCS([snprintf strncat],,AC_MSG_ERROR(Cannot find all the necessary functions. Try to install the necessary libraries and headers))
    AC_FUNC_MALLOC(,,AC_MSG_ERROR(Cannot find all the necessary functions. Try to install the necessary libraries and headers))
    #AC_FUNC_MKTIME(,,AC_MSG_ERROR(Cannot find all the necessary functions. Try to install the necessary libraries and headers))
    AC_FUNC_REALLOC(,,AC_MSG_ERROR(Cannot find all the necessary functions. Try to install the necessary libraries and headers))

],[
    AC_MSG_NOTICE([Attempting to compile for a Windows target.]) 
    # >> Windows

    # - check compiler first
    compileros=`echo $CC | grep mingw`
    AS_IF([test -z "$compileros"], [
    AC_WARN([You have chosen a mingw target but apparently the compiler is not a mingw cross-compiler. Please fix this if you are on Unix trying to cross-compile for Windows (if you are on Windows, everything should be fine).])
    ])

    TGHOST="$host_alias"
    AC_SUBST(TGHOST)
    
    WRITELINE="\nEXEEXT=$EXEEXT\nluatarget=mingw\nCC=$CC\nAR=$AR\nHOST=$TGHOST\nRANLIB=$RANLIB\nSTATIC_LIBS_USE=$STATIC_LIBS_USE\nMACBUILD=no"
    echo -e "$WRITELINE" > scripts/.buildinfo

    CFLAGS="$CFLAGS -static-libgcc -static-libstdc++ -mwindows -mthread -mthreads -L../ -I../ `cd scripts && sh ./checksse.sh`"
    CXXFLAGS="$CXXFLAGS -static-libgcc -static-libstdc++ -mwindows -mthread -mthreads -L../ -I../ `cd scripts && sh ./checksse.sh`"
    LIBS="$LIBS -lshlwapi -lgdi32 -limm32 -loleaut32 -lole32 -lwinmm -luuid -lversion"

    AC_CHECK_HEADERS([windows.h string.h stdio.h unistd.h],,AC_MSG_ERROR(Cannot find all the necessary headers. Try to install the necessary libraries and headers))

])

# Check whether we can access cmake
AC_MSG_CHECKING([for cmake build system])
sh scripts/checkforcmake.sh
AS_IF([test "x$?" != x0],[
    AC_MSG_RESULT([no])
    AC_MSG_ERROR([cmake was not found on your machine.])
    exit 1;
],[
    AC_MSG_RESULT([yes])
])

AC_CONFIG_FILES([Makefile
         src/Makefile])
AC_OUTPUT


# Print final deps info:

AC_MSG_NOTICE([${LIBS_DEP_INFO_TEXT}])
echo "
This results in the following feature set:
  Graphics: ${GRAPHICS_FEATURE_ENABLED}
  Null device graphics: ${NULL_GRAPHICS_FEATURE_ENABLED}
  Audio: ${AUDIO_FEATURE_ENABLED}
  Null device audio: ${NULL_AUDIO_FEATURE_ENABLED}
  Flac decoding: ${FLAC_FEATURE_ENABLED}
  Speex-based resampling: ${SPEEX_RESAMPLING_FEATURE_ENABLED}
  Runtime FFmpeg support: ${FFMPEG_FEATURE_ENABLED}
  Physics simulation support: ${PHYSICS_FEATURE_ENABLED}
"


# Compose ld/include/define flags:

FINAL_LD_FLAGS=""
FINAL_INCLUDE_FLAGS="-Iimgloader/"
FINAL_USE_LIB_FLAGS=""

SDLADDED=no
AS_IF([test "x$GRAPHICS_FEATURE_ENABLED" = xyes],[
    statictest=`echo $STATIC_LIBS_USE | grep SDL2`
    SDLADDED=yes
    AS_IF([test -z "$statictest"], [
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lSDL2"
    ], [
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardSDL"
        FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Isdl/include"
    ])
    FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DUSE_SDL_GRAPHICS"
])
AS_IF([test "x$AUDIO_FEATURE_ENABLED" = xyes],[
    AS_IF([test "x$SDLADDED" = xno],[
        SDLADDED=yes
        AS_IF([test -z "$statictest"], [
            FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lSDL2"
        ], [
            FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardSDL"
            FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Isdl/include"
        ])
    ])
    FINAL_USE_LIBS_FLAGS="$FINAL_USE_LIB_FLAGS -DUSE_SDL_AUDIO"
])

AS_IF([test "x$NULL_GRAPHICS_FEATURE_ENABLED" = xyes],[
    FINAL_LD_FLAGS="$FINAL_LD_FLAGS -limglib"
    statictest=`echo $STATIC_LIBS_USE | grep png`
    AS_IF([test -z "$statictest"],[
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lpng"
    ], [
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardpng"
    ])
    AS_IF([test -z "$statictest"],[
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lz"
    ], [
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardzlib"
    ])
    FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DUSE_GRAPHICS"
])

AS_IF([test "x$NULL_AUDIO_FEATURE_ENABLED" = xyes],[
    statictest=`echo $STATIC_LIBS_USE | grep vorbis`
    AS_IF([test -z "$statictest"],[
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lvorbisfile -lvorbis"
    ],[
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardvorbisfile -lblitwizardvorbis"
        FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Ivorbis/include"

    ])
    statictest=`echo $STATIC_LIBS_USE | grep ogg`
    AS_IF([test -z "$statictest"],[
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -logg"
    ], [
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardogg"
        FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Iogg/include"
    ])
    FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DUSE_AUDIO"
])

statictest=`echo $STATIC_LIBS_USE | grep lua`
AS_IF([test -z "$statictest"],[
    FINAL_LD_FLAGS="$FINAL_LD_FLAGS -llua"
], [
    FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardlua"
    FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Ilua/src"
])

AS_IF([test "x$NULL_AUDIO_FEATURE_ENABLED" = xyes],[
    AS_IF([test "x$FLAC_FEATURE_ENABLED" = xyes],[
        statictest=`echo $STATIC_LIBS_USE | grep FLAC`
        AS_IF([test -z "$statictest"],[
            FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lFLAC"
        ], [
            FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardFLAC"
            FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Iflac/include"
        ])
        FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DUSE_FLAC_AUDIO"
    ])

    AS_IF([test "x$SPEEX_RESAMPLING_FEATURE_ENABLED" = xyes],[
        statictest=`echo $STATIC_LIBS_USE | grep FLAC`
        AS_IF([test -z "$statictest"],[
            FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lspeex -lspeexdsp"
        ], [
            FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardspeex -lblitwizardspeexdsp"
            FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Ispeex/include"
        ])
        FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DUSE_SPEEX_RESAMPLING"
    ])

    AS_IF([test "x$FFMPEG_FEATURE_ENABLED" = xyes],[
        FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Iffmpeg/"
        FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DUSE_FFMEG_AUDIO"
    ])

])

AS_IF([test "x$PHYSICS_FEATURE_ENABLED" = xyes],[
    statictest=`echo $STATIC_LIBS_USE | grep box2d`
    AS_IF([test -z "$statictest"],[
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lbox2d"
    ], [
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardbox2d"
        FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Ibox2d/"
    ])
    FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DUSE_PHYSICS"
])


# Echo the build flags

echo "
Build flags:
  LD flags:      $FINAL_LD_FLAGS
  include flags: $FINAL_INCLUDE_FLAGS
  Feature flags: $FINAL_USE_LIB_FLAGS"


# Conserve the build flags for the make script

echo "$FINAL_LD_FLAGS" > scripts/.ldflags
echo "$FINAL_INCLUDE_FLAGS" > scripts/.includeflags
echo "$FINAL_USE_LIB_FLAGS" > scripts/.uselibflags


