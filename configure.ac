#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([prpltwtr], [0.6.3], [http://code.google.com/p/prpltwtr/issues/list])
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE([1.9.5])
AM_MAINTAINER_MODE

REQUIRED_PKG_CONFIG="0.15.0"
AC_SUBST(REQUIRED_PKG_CONFIG)

REQUIRED_PIDGIN="pidgin >= 2.6 pidgin < 3.0"
AC_SUBST(REQUIRED_PIDGIN)

AC_CONFIG_HEADER([config.h])

AC_DEFINE(PURPLE_PLUGINS, 1, [Define if plugins are enabled.])

# Checks for programs.
AC_PROG_CC
AC_DISABLE_STATIC
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
LIBTOOL="$LIBTOOL --silent"
AC_PROG_INSTALL
AC_PATH_PROG(PKG_CONFIG, [pkg-config], [no])

LIBPURPLE_DATADIR=`pkg-config --variable=datadir purple`
AC_SUBST(LIBPURPLE_DATADIR)

if test x$PKG_CONFIG = xno ; then
  AC_MSG_ERROR([*** pkg-config not found])
fi

# Checks for libraries.
PKG_PROG_PKG_CONFIG([$REQUIRED_PKG_CONFIG])
PKG_CHECK_MODULES([PIDGIN], [$REQUIRED_PIDGIN],
	[
	AC_SUBST(PIDGIN_CFLAGS)
	AC_SUBST(PIDGIN_LIBS)
	]
)
AC_ARG_VAR([PIDGIN_DATADIR], [datadir of PIDGIN, overriding pkg-config])dnl
AC_ARG_VAR([PIDGIN_PLUGINDIR], [plugindir of PIDGIN, overriding pkg-config])dnl
AC_CACHE_CHECK([for PIDGIN][_DATADIR], [pkg_cv_][PIDGIN][_DATADIR],
	[_PKG_CONFIG([PIDGIN][_DATADIR], [variable=datadir], [pidgin])])
AC_CACHE_CHECK([for PIDGIN][_PLUGINDIR], [pkg_cv_][PIDGIN][_PLUGINDIR],
	[_PKG_CONFIG([PIDGIN][_PLUGINDIR], [variable=plugindir], [pidgin])])
PIDGIN[]_DATADIR=$pkg_cv_[]PIDGIN[]_DATADIR
PIDGIN[]_PLUGINDIR=$pkg_cv_[]PIDGIN[]_PLUGINDIR
AC_SUBST(PIDGIN_DATADIR)
AC_SUBST(PIDGIN_PLUGINDIR)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h])

# AM_CONFIG_HEADER

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([gethostname inet_ntoa memmove strcasecmp strchr strdup strerror strpbrk strrchr strstr strtoul])

case "$host" in
	*-*-mingw*)
	CFLAGS="$CFLAGS -O2 -Wall -mno-cygwin -mms-bitfields"
	LIBS="$LIBS -no-undefined -Wl,--enable-auto-image-base -lws2_32"
	;;
	*)
	;;
esac

if test "x$GCC" = "xyes"; then
	CFLAGS="$CFLAGS -Wall -g3"
fi
AC_SUBST(CFLAGS)

# Creating files.
AC_OUTPUT([
Makefile
data/Makefile
src/Makefile
src/prpltwtr/Makefile
src/gtkprpltwtr/Makefile
])

echo
echo $PACKAGE $VERSION
echo
echo Installation directories: $PIDGIN_PLUGINDIR and $PIDGIN_DATADIR
echo If you want to override, rerun ./configure with the parameters PIDGIN_PLUGINDIR and PIDGIN_DATADIR set appropriately.
echo
echo next steps:
echo make
echo sudo make install
echo
