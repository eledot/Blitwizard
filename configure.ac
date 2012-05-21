
# blitwizard configure.ac - autoconf generated, extended by Jonas Thiem
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])

AC_INIT([blitwizard], [0.3])

# Checks for programs:

AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_MAKE_SET
MACTEST=`uname -a | grep Darwin`
AS_IF([test -z "$MACTEST"],[
AC_PROG_OBJC
])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])


# Check for ar:

AC_CHECK_TOOL(AR, ar)
if [ test -z "$AR" ]; then
    AC_MSG_ERROR([No ar available. Please install ar.])
    exit 1
fi


# Enable/disable features:

AC_ARG_ENABLE([sdl-graphics],
    [AS_HELP_STRING([--disable-sdl-graphics],[disable graphics support through SDL (default is enabled)])],
[case "${enableval}" in
    yes | no ) enable_sdlgraphics="${enableval}" ;;
    *) AC_MSG_ERROR(bad value ${enableval} for --disable-sdl-graphics) ;;
esac],
[enable_sdlgraphics=yes])

AC_ARG_ENABLE([sdl-audio],
    [AS_HELP_STRING([--disable-sdl-audio],[disable audio support through SDL (default is enabled)])],
[case "${enableval}" in
     yes | no ) enable_sdlaudio="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-sdl-audio) ;;
 esac],
[enable_sdlaudio=yes])

AC_ARG_ENABLE([sdl],
    [AS_HELP_STRING([--disable-sdl],[disable SDL completely. This is the same as --disable-sdl-audio, --disable-sdl-graphics combined])],
[case "${enableval}" in
     yes | no ) enable_sdl="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-sdl) ;;
 esac],
[enable_sdl=yes])

AS_IF([test "x$enable_sdl" = xno], [
    enable_sdlaudio="no"
    enable_sdlgraphics="no"
])

AC_ARG_ENABLE([graphics],
    [AS_HELP_STRING([--disable-graphics],[disable graphics support completely, including the null graphics device (default is enabled)])],
[case "${enableval}" in
     yes | no ) enable_graphics="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-graphics) ;;
 esac],
[enable_graphics=yes])

AC_ARG_ENABLE([audio],
    [AS_HELP_STRING([--disable-audio],[disable audio support completely, including the null audio device (default is enabled)])],
[case "${enableval}" in
     yes | no ) enable_audio="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-audio) ;;
 esac],
[enable_audio="yes"])

AC_ARG_ENABLE([physics],
    [AS_HELP_STRING([--disable-physics],[disable physics simulation support (default is enabled)])],
[case "${enableval}" in
     yes | no ) enable_physics="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-physics) ;;
 esac],
[enable_physics=yes])

AC_ARG_ENABLE([flac],
    [AS_HELP_STRING([--disable-flac],[disable optional FLAC audio support.])],
[case "${enableval}" in
     yes | no ) enable_flac="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-flac) ;;
 esac],
[enable_flac=yes])

AC_ARG_ENABLE([speex],
    [AS_HELP_STRING([--disable-speex],[disable optional Speex audio support.])],
[case "${enableval}" in
     yes | no ) enable_speex="${enableval}" ;;
     *) AC_MSG_ERROR(bad value ${enableval} for --disable-speex) ;;
 esac],
[enable_speex=yes])


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
FAST_FLOAT2INT_FEATURE_ENABLED="yes"


# Lib deps info text for later output:

LIBS_DEP_INFO_TEXT="
*** Dependency scan results ***"


# Check whether ffmpeg is available

AS_IF([test -e "src/ffmpeg/libavcodec/avcodec.h"],[
    AC_MSG_NOTICE([FFmpeg source code appears to be present, will compile with FFmpeg support.])
    FFMPEG_CONFIG_FAILED="no"
    AS_IF([test ! -e "src/ffmpeg/libavutil/avconfig.h"], [
        AC_WARN([FFmpeg is not configured yet. FFmpeg will be now configured (but not built) with recommended options. If you want to do that yourself, please cancel and do it now, then rerun blitwizard ./configure.])
        CONFIGUREFFMPEG="cd src/ffmpeg && ./configure --disable-libgsm --enable-runtime-cpudetect  --enable-version3 --disable-gpl --enable-shared --disable-doc --disable-ffmpeg --disable-avconv --disable-ffplay --disable-ffprobe --disable-ffserver --disable-protocols --disable-devices;"
        eval $CONFIGUREFFMPEG || { FFMPEG_CONFIG_FAILED="yes"; }
        cd ../../
    ])
    AS_IF([test "x$FFMPEG_CONFIG_FAILED" = xno],[
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  FFmpeg headers from tarball: yes"
    ],[
        AC_WARN([Failed to configure FFmpeg. Blitwizard will NOT have any FFmpeg support])
        LIBS_DEP_INFO_TEXT="${LIPS_DEP_INFO_TEXT}
  FFmpeg headers from tarball: no"
        FFMPEG_FEATURE_ENABLED="no, FFmpeg configure in tarball failed" 
    ])
],[
    AC_WARN([FFmpeg source code is not present or not properly placed. Blitwizard will NOT have any FFmpeg support.])
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  FFmpeg headers from tarball: no"
    FFMPEG_FEATURE_ENABLED="no, tarball not found"
])


# Check for SDL2 availability:

AS_IF([test "x$enable_audio" = xno], [
    enable_sdlaudio="no"
])
AS_IF([test "x$enable_graphics" = xno], [
    enable_sdlgraphics="no"
])

NEEDSDL="no"
AS_IF([test "x$enable_sdlaudio" = xyes], [
    NEEDSDL="yes"
])
AS_IF([test "x$enable_sdlgraphics" = xyes], [
    NEEDSDL="yes"
])

AS_IF([test "x$NEEDSDL" = xyes],[
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
])

AS_IF([test "x$enable_sdlgraphics" = xno],[
    GRAPHICS_FEATURE_ENABLED="no, disabled"
])
AS_IF([test "x$enable_sdlaudio" = xno],[
    AUDIO_FEATURE_ENABLED="no, disabled"
])

AS_IF([test "x$enable_graphics" = xyes],[ # START enable_graphics block

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
        GRAPHICS_FEATURE_ENABLED="no, need zlib"
        NULL_GRAPHICS_FEATURE_ENABLED="no, need zlib"
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} zlib"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  zlib: static"
])

],[ # STOP enable_graphics block
    GRAPHICS_FEATURE_ENABLED="no, disabled"
    NULL_GRAPHICS_FEATURE_ENABLED="no, disabled"
])

AS_IF([test "x$enable_audio" = xyes],[ # START enable_audio block

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
            FFMPEG_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
            SPEEX_RESAMPLING_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
            FLAC_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
        ])
    ],[
        #nothing there
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libvorbis/vorbisfile: no"
        AUDIO_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
        NULL_AUDIO_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
        FFMPEG_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
        SPEEX_RESAMPLING_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
        FLAC_FEATURE_ENABLED="no, need libvorbis/libvorbisfile"
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
        FFMPEG_FEATURE_ENABLED="no, need libogg"
        SPEEX_RESAMPLING_FEATURE_ENABLED="no, need libogg"
        FLAC_FEATURE_ENABLED="no, need libogg"
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} ogg"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libogg: static"
])


# Check for libFLAC availability:

AS_IF([test "x$enable_flac" = xyes],[
    AC_MSG_CHECKING([for static libFLAC library])
    AS_IF([test ! -f "src/flac/src/libFLAC/lpc.c"],[
        AC_MSG_RESULT([no])

        # Check for shared FLAC lib:
        HAVESHAREDFLAC="no"
        AC_CHECK_LIB([FLAC], [FLAC__stream_decoder_new],[
            AC_CHECK_HEADER([FLAC/format.h], [
                HAVESHAREDFLAC="yes"
                LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libFLAC: shared"
            ])
        ])

        AS_IF([test "x$HAVESHAREDFLAC" = xno],[
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
], [
    FLAC_FEATURE_ENABLED="no, disabled"
])


# Check for libspeex availability:

AS_IF([test "x$enable_speex" = xyes],[
    AC_MSG_CHECKING([for static libspeex library])
    AS_IF([test ! -f "src/speex/src/speexdec.c"],[
        AC_MSG_RESULT([no])

        # Check for shared speex lib:
        HAVESHAREDSPEEX="no"
        AC_CHECK_LIB([speexdsp], [speex_resampler_init],[
            AC_CHECK_LIB([speex], [speex_encoder_init],[
                AC_CHECK_HEADER([speex/speex_resampler.h], [
                    HAVESHAREDSPEEX="yes"
                    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libspeex: shared"
                ])
            ])
        ])

        # Deactivate features if necessary:
        AS_IF([test "x$HAVESHAREDSPEEX" = xno],[
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
],[
    SPEEX_RESAMPLING_FEATURE_ENABLED="no, disabled"
])

],[ # STOP enable_audio block
    AUDIO_FEATURE_ENABLED="no, disabled"
    NULL_AUDIO_FEATURE_ENABLED="no, disabled"
    FLAC_FEATURE_ENABLED="no, disabled"
    SPEEX_RESAMPLING_FEATURE_ENABLED="no, disabled"
    FFMPEG_FEATURE_ENABLED="no, disabled"
])


# Check for liblua availability:

AC_MSG_CHECKING([for static lua library])
AS_IF([test ! -f "src/lua/src/lua.h"],[
    AC_MSG_RESULT([no])
    HAVE_LUA_SHARED="no"

    # Check for regular -llua5.2 first
    AC_CHECK_LIB([lua5.2], [lua_close],[
        AC_CHECK_LIB([lua5.2], [lua_rawlen],[
            AC_CHECK_HEADER([lua5.2/lua.h], [
                #shared lib is there!
                LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  liblua: shared"
                HAVE_LUA_SHARED="yes"
                SHARED_LUA_5_2="yes"
                FAST_FLOAT2INT_FEATURE_ENABLED="no, need static liblua"
            ])
        ])
    ])

    # Check for generic -llua
    AS_IF([test "x$HAVE_LUA_SHARED" = xno],[
        AC_CHECK_LIB([lua], [lua_close],[
            AC_CHECK_LIB([lua], [lua_rawlen],[
                AC_CHECK_HEADER([lua.h], [
                    #shared lib is there!
                    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  liblua: shared"
                    HAVE_LUA_SHARED="yes"
                    FAST_FLOAT2INT_FEATURE_ENABLED="no, need static liblua"
                ])
            ])
        ])
    ])

    AS_IF([test "x$HAVE_LUA_SHARED" = xno],[
        #nothing there
        AC_MSG_ERROR([Lua 5.2 is required for blitwizard. Check README-deps.txt for help on how to add it.])
    ])
],[
    AC_MSG_RESULT([yes])
    STATIC_LIBS_USE="${STATIC_LIBS_USE} lua"
    LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  liblua: static"
])


# Check for libbox2d availability:

AS_IF([test "x$enable_physics" = xyes],[
    AC_MSG_CHECKING([for static Box2D library])
    HAVE_STATIC_BOX2D="yes"
    AS_IF([test ! -f "src/box2d/Box2D/Box2D.h"],[
        AC_MSG_RESULT([no])
        HAVE_STATIC_BOX2D="no"
    ], [
        AC_MSG_RESULT([yes])
    ])
    AS_IF([test "x$HAVE_STATIC_BOX2D" = xno], [
        #check for shared lib
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
        #static lib is here
        STATIC_LIBS_USE="${STATIC_LIBS_USE} box2d"
        LIBS_DEP_INFO_TEXT="${LIBS_DEP_INFO_TEXT}
  libbox2d: static"
    ])
],[
    PHYSICS_FEATURE_ENABLED="no, disabled"
])

# Prepare final deps info output:

DEPS_INFO_OUTPUT="
This results in the following feature set:
  Graphics: ${GRAPHICS_FEATURE_ENABLED}
  Null device graphics: ${NULL_GRAPHICS_FEATURE_ENABLED}
  Audio: ${AUDIO_FEATURE_ENABLED}
  Null device audio: ${NULL_AUDIO_FEATURE_ENABLED}
  Flac decoding: ${FLAC_FEATURE_ENABLED}
  Speex-based resampling: ${SPEEX_RESAMPLING_FEATURE_ENABLED}
  Runtime FFmpeg support: ${FFMPEG_FEATURE_ENABLED}
  Physics simulation support: ${PHYSICS_FEATURE_ENABLED}
  Fast float-to-int conversion: ${FAST_FLOAT2INT_FEATURE_ENABLED}
"


# Compose ld/include/define flags:

FINAL_LD_FLAGS=""
FINAL_INCLUDE_FLAGS=""
FINAL_USE_LIB_FLAGS=""

SDLADDED=no
statictest=`echo $STATIC_LIBS_USE | grep lua`
AS_IF([test -z "$statictest"], [
    FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DNOLLIMITS"
    AS_IF([test "x$SHARED_LUA_5_2" = xyes],[
        FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DLUA_5_2_HEADER"
    ])
])
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
        statictest=`echo $STATIC_LIBS_USE | grep SDL2`
        SDLADDED=yes
        AS_IF([test -z "$statictest"], [
            FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lSDL2"
        ], [
            FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardSDL"
            FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Isdl/include"
        ])
    ])
    FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DUSE_SDL_AUDIO"
])

AS_IF([test "x$NULL_GRAPHICS_FEATURE_ENABLED" = xyes],[
    FINAL_LD_FLAGS="$FINAL_LD_FLAGS -limglib"
    FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Iimgloader/"
    STATIC_LIBS_USE="$STATIC_LIBS_USE imgloader"
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
    AS_IF([test "x$SHARED_LUA_5_2" = xyes],[
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -llua5.2"
    ],[
        FINAL_LD_FLAGS="$FINAL_LD_FLAGS -llua"
    ])
], [
    FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardlua"
    FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Ilua/src"
])

AS_IF([test "x$NULL_AUDIO_FEATURE_ENABLED" = xyes],[

    # Checking on FLAC status:
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

    # Checking on Speex status:
    AS_IF([test "x$SPEEX_RESAMPLING_FEATURE_ENABLED" = xyes],[
        statictest=`echo $STATIC_LIBS_USE | grep speex`
        AS_IF([test -z "$statictest"],[
            FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lspeex -lspeexdsp"
        ], [
            FINAL_LD_FLAGS="$FINAL_LD_FLAGS -lblitwizardspeex -lblitwizardspeexdsp"
            FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Ispeex/include"
        ])
        FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DUSE_SPEEX_RESAMPLING"
    ])

    # Checking on FFmpeg status:
    AS_IF([test "x$FFMPEG_FEATURE_ENABLED" = xyes],[
        FINAL_INCLUDE_FLAGS="$FINAL_INCLUDE_FLAGS -Iffmpeg/"
        FINAL_USE_LIB_FLAGS="$FINAL_USE_LIB_FLAGS -DUSE_FFMPEG_AUDIO"
    ])

]) # AUDIO_FEATURE_ENABLED

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

# Get the scripts/ dir (absolute path):
cd scripts/
scriptsdir="`pwd`"
cd ..


# OS dependant things:
AS_IF([test -z "$maybeos"],[
    AC_MSG_NOTICE([Trying to compile for a Unix target.]) 
    # >> Unix

    MACTEST=`uname -a | grep Darwin`
    AS_IF([test -z "$MACTEST"],[
        # Non-Mac Unix:

        AM_CONDITIONAL([MAC_APP_PATHS], [test 0 = 1])
        LIBS="-pthread -ldl -lm -lrt"
        WRITELINE="\nluatarget=posix\nCXX=$CXX\nCC=$CC\nAR=$AR\nSTATIC_LIBS_USE=$STATIC_LIBS_USE\nHOST=$TGHOST\nUSE_LIB_FLAGS=$FINAL_USE_LIB_FLAGS\nRANLIB=$RANLIB\nMACBUILD=no"
    ],[
        # Mac OS X:

        # Use this compiler since the default one hates libvorbis:
        CC="clang -arch i386 -arch i386 "
        # Also, we want to compile a 32bit bin so we can load FFmpeg

        # Various other flags:
        AM_CONDITIONAL([MAC_APP_PATHS], [test 1 = 1])
        LIBS="$LIBS -lm -liconv -pthread -framework CoreServices -framework Carbon -framework AudioUnit -framework ForceFeedback -framework OpenGL -framework AppKit -framework Cocoa -framework Foundation -framework IOKit -framework CoreAudio"
        WRITELINE="\nEXEEXT=$EXEEXT\nluatarget=posix\nCXX=$CXX\nCC=$CC\nSTATIC_LIBS_USE=$STATIC_LIBS_USE\nAR=$AR\nHOST=$TGHOST\nUSE_LIB_FLAGS=$FINAL_USE_LIB_FLAGS\nRANLIB=$RANLIB\nMACBUILD=yes"
    ])

    echo -e "$WRITELINE" > scripts/.buildinfo
    CFLAGS="$CFLAGS $USE_FFMPEG `cd $scriptsdir && sh ./checksse.sh`"   

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

    AM_CONDITIONAL([MAC_APP_PATHS], [test 0 = 1])

    # - check compiler first
    compileros=`echo $CC | grep mingw`
    AS_IF([test -z "$compileros"], [
    AC_WARN([You have chosen a mingw target but apparently the compiler is not a mingw cross-compiler. Please fix this if you are on Unix trying to cross-compile for Windows (if you are on Windows, everything should be fine).])
    ])

    TGHOST="$host_alias"
    AC_SUBST(TGHOST)
    
    WRITELINE="\nEXEEXT=$EXEEXT\nluatarget=mingw\nCXX=$CXX\nCC=$CC\nAR=$AR\nHOST=$TGHOST\nUSE_LIB_FLAGS=$FINAL_USE_LIB_FLAGS\nRANLIB=$RANLIB\nSTATIC_LIBS_USE=$STATIC_LIBS_USE\nMACBUILD=no"
    echo -e "$WRITELINE" > scripts/.buildinfo

    CFLAGS="$CFLAGS -static-libgcc -mwindows -mthreads -L../ -I../ `cd $scriptsdir && sh ./checksse.sh`"
    CXXFLAGS="$CXXFLAGS -static-libgcc -static-libstdc++ -mwindows -mthreads -L../ -I../ `cd $scriptsdir && sh ./checksse.sh`"
    LIBS="$LIBS -lshlwapi -lgdi32 -limm32 -loleaut32 -lws2_32 -lole32 -lwinmm -luuid -lversion"
    FINAL_LD_FLAGS="-lmingw32 $FINAL_LD_FLAGS"
    STATICSTDCXX="-static-libstdc++"
    AC_SUBST(STATICSTDCXX)

    AC_CHECK_HEADERS([windows.h string.h stdio.h unistd.h],,AC_MSG_ERROR(Cannot find all the necessary headers. Try to install the necessary libraries and headers))

])


# Prepare output of the build flags

BUILD_FLAGS_OUTPUT="
Build flags:
  LD flags:      $FINAL_LD_FLAGS
  include flags: $FINAL_INCLUDE_FLAGS
  Feature flags: $FINAL_USE_LIB_FLAGS"


# Add the build flags for the make script

AC_SUBST(FINAL_INCLUDE_FLAGS)
AC_SUBST(FINAL_USE_LIB_FLAGS)
AC_SUBST(FINAL_LD_FLAGS)

CFLAGS="$FINAL_INCLUDE_FLAGS $CFLAGS $FINAL_USE_LIB_FLAGS"
CXXFLAGS="$FINAL_INCLUDE_FLAGS $CFLAGS $FINAL_USE_LIB_FLAGS"
LIBS="$FINAL_LD_FLAGS $LIBS"



AC_CONFIG_FILES([Makefile
                 src/Makefile
                 tests/Makefile])
AC_OUTPUT

echo "$LIBS_DEP_INFO_TEXT"
echo "$DEPS_INFO_OUTPUT"

echo " "
echo "*** You can now build the project by typing 'make'."
echo "The resulting binaries will be created inside the bin/ folder."

