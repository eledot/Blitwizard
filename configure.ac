
# blitwizard configure.ac - autoconf generated, extended by Jonas Thiem
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])

AC_INIT([blitwizard], [0.1])

# Checks for programs.
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC
AC_PROG_MAKE_SET
AC_CANONICAL_HOST

AC_CONFIG_MACRO_DIR([m4])


# --> EVALUATE TARGET OS

maybeos=`echo $EXEEXT | grep exe`
# Checks for libraries.


# --- OS DEPENDENT STUFF ---

AS_IF([test -z "$maybeos"],[
	AC_MSG_NOTICE([Trying to compile for a Unix target.]) 
	# >> Unix

	CFLAGS="$CFLAGS -rdynamic"
	LIBS="-lpthread -ldl"
	
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
	AC_WARN([You have chosen a mingw target but apparently the compiler is not a mingw cross-compiler. Please fix this if you are on Unix trying to cross-compile for Windows (if you are on Windows, everything is fine).])
	])

	TGHOST="$host_alias"
	AC_SUBST(TGHOST)

	CFLAGS="$CFLAGS -D_WIN32_WINNT=0x0501 -DWIN -mconsole -L../ -I../"
	

	AC_CHECK_HEADERS([windows.h string.h stdio.h unistd.h],,AC_MSG_ERROR(Cannot find all the necessary headers. Try to install the necessary libraries and headers))

])


AC_CONFIG_FILES([Makefile
		 src/Makefile])
AC_OUTPUT

