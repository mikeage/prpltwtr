#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([prpltwtr], [0.12.0], [http://code.google.com/p/prpltwtr/issues/list])
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE([1.9.5])

REQUIRED_PKG_CONFIG="0.15.0"
AC_SUBST(REQUIRED_PKG_CONFIG)

AC_ARG_WITH([pidgin],
			[AS_HELP_STRING([--without-pidgin],
							[disable support for pidgin])],
							[],
							[with_pidgin=yes])


AC_CONFIG_SRCDIR([src/Makefile.mingw])
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

#
# Setup gettext
#
AC_PROG_INTLTOOL
GETTEXT_PACKAGE=prpltwtr
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, ["$GETTEXT_PACKAGE"], [Define the gettext package to be used])
ALL_LINGUAS="es ja"
AM_GLIB_GNU_GETTEXT

if test x$PKG_CONFIG = xno ; then
  AC_MSG_ERROR([*** pkg-config not found])
fi

# Checks for libraries.
PKG_PROG_PKG_CONFIG([$REQUIRED_PKG_CONFIG])
AS_IF([test "x$with_pidgin" != xno],
	  [PKG_CHECK_MODULES([PIDGIN], [pidgin >= 2.6 pidgin < 3.0],
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
	  ],[]
	  )

AM_CONDITIONAL([WITH_PIDGIN], [test "x$with_pidgin" != xno])

PKG_CHECK_MODULES([PURPLE], [purple],
				  [
				   AC_SUBST(PURPLE_CFLAGS)
				   AC_SUBST(PURPLE_LIBS)
				   ]
				  )

AC_ARG_VAR([PURPLE_PLUGINDIR], [plugindir of PURPLE, overriding pkg-config])dnl
AC_CACHE_CHECK([for PURPLE][_DATADIR], [pkg_cv_][PURPLE][_DATADIR],
			   [_PKG_CONFIG([PURPLE][_DATADIR], [variable=datadir], [purple])])
AC_CACHE_CHECK([for PURPLE][_PLUGINDIR], [pkg_cv_][PURPLE][_PLUGINDIR],
			   [_PKG_CONFIG([PURPLE][_PLUGINDIR], [variable=plugindir], [purple])])
PURPLE[]_DATADIR=$pkg_cv_[]PURPLE[]_DATADIR
PURPLE[]_PLUGINDIR=$pkg_cv_[]PURPLE[]_PLUGINDIR
AC_SUBST(PURPLE_PLUGINDIR)

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
AM_MAINTAINER_MODE

# Creating files.
AC_CONFIG_FILES([Makefile
				 po/Makefile.in
				 data/Makefile
				 src/Makefile
				 src/prpltwtr/Makefile
				 src/gtkprpltwtr/Makefile])
AC_OUTPUT

echo
echo $PACKAGE $VERSION
echo
AS_IF([test "x$with_pidgin" != xno],
[
 echo Pidgin plugin directory: $PIDGIN_PLUGINDIR . Override by setting PIDGIN_PLUGINDIR
 echo Pidgin data directory: $PIDGIN_DATADIR . Override by setting PIDGIN_DATADIR 
],[])
echo libpurple plugin directory: $PURPLE_PLUGINDIR . Override by setting PURPLE_PLUGINDIR
echo
echo next steps:
echo make
echo sudo make install
echo
