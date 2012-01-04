
# blitwizard configure.ac - autoconf generated, extended by Jonas Thiem
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])

AC_INIT([blitwizard], [0.1])

# Checks for programs.
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_MAKE_SET
AC_CANONICAL_HOST

AC_DISABLE_SHARED
AC_PROG_LIBTOOL

AC_CONFIG_MACRO_DIR([m4])


# --> EVALUATE TARGET OS

maybeos=`echo $EXEEXT | grep exe`
# Checks for libraries.


# Check whether ffmpeg is available

USE_FFMPEG=""
AS_IF([test -e "src/ffmpeg/libavcodec/avcodec.h"],[
	AC_MSG_NOTICE([FFmpeg source code appears to be present, will compile with FFmpeg support.])
	USE_FFMPEG="-DUSE_FFMPEG"
],[
	AC_WARN([FFmpeg source code is not present or not properly placed. Blitwizard will NOT have any FFmpeg support.])
])


# Os dependent stuff:

AS_IF([test -z "$maybeos"],[
	AC_MSG_NOTICE([Trying to compile for a Unix target.]) 
	# >> Unix

	MACTEST=`uname -a | grep Darwin`
	AS_IF([test -z "$MACTEST"],[
		LIBS="-lpthread -ldl -lm"
        WRITELINE="\nluatarget=posix\nCC=$CC\nAR=$AR\nHOST=$TGHOST\nRANLIB=$RANLIB\nMACBUILD=no"
	],[
		LIBS="$LIBS -lpthread -framework CoreServices -framework Carbon -framework AudioUnit -framework ForceFeedback -framework OpenGL -framework AppKit -framework Cocoa -framework Foundation -framework IOKit -framework CoreAudio"
        WRITELINE="\nEXEEXT=$EXEEXT\nluatarget=posix\nCC=$CC\nAR=$AR\nHOST=$TGHOST\nRANLIB=$RANLIB\nMACBUILD=yes"
	])

	echo -e "$WRITELINE" > scripts/.buildinfo
	CFLAGS="$CFLAGS $USE_FFMPEG `cd scripts && sh ./checksse.sh`"	

	AC_CHECK_LIB([dl], [dlopen],, AC_MSG_ERROR(Cannot find all the necessary functions. Try to install the necessary libraries and headers))
	#AC_CHECK_LIB([sqlite3], [sqlite3_prepare_v2],, AC_MSG_ERROR(Cannot find all the necessary functions. Try to install the necessary libraries and headers))

	# Checks for header files.
	AC_CHECK_HEADERS([arpa/inet.h dlfcn.h ctype.h fcntl.h netdb.h netinet/in.h stdarg.h sys/stat.h sys/socket.h sys/wait.h],,AC_MSG_ERROR(Cannot find all the necessary headers. Try to install the necessary libraries and headers))
	AC_CHECK_HEADERS([malloc.h])
	#AC_CHECK_HEADERS([sqlite3.h],,AC_MSG_ERROR(Cannot find all the necessary headers. Try to install the necessary libraries and headers))


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
	
	WRITELINE="\nEXEEXT=$EXEEXT\nluatarget=mingw\nCC=$CC\nAR=$AR\nHOST=$TGHOST\nRANLIB=$RANLIB\nMACBUILD=no"
	echo -e "$WRITELINE" > scripts/.buildinfo

	CFLAGS="$CFLAGS -D_WIN32_WINNT=0x0501 -DWIN -mwindows -L../ -I../ `cd scripts && sh ./checksse.sh`"
	LIBS="$LIBS -lshlwapi -lgdi32 -limm32 -loleaut32 -lole32 -lwinmm -luuid -lversion"

	AC_CHECK_HEADERS([windows.h string.h stdio.h unistd.h],,AC_MSG_ERROR(Cannot find all the necessary headers. Try to install the necessary libraries and headers))

])


AC_CONFIG_FILES([Makefile
		 src/Makefile])
AC_OUTPUT

