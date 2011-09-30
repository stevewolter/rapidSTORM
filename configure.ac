#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([rapidSTORM], [4.0.0], [Steve Wolter <rapidstorm@swolter.sdf1.org>])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([dstorm.cpp])
AC_CONFIG_HEADERS([config.h])

AC_PROG_SED
test "x$SED" = "x" && AC_MSG_ERROR([unable to find stream editor])

AX_SPLIT_VERSION
AC_DEFINE_UNQUOTED([PACKAGE_MAJOR_VERSION], $AX_MAJOR_VERSION, [Major part of version number])
AC_DEFINE_UNQUOTED([PACKAGE_MINOR_VERSION], $AX_MINOR_VERSION, [Minor part of version number])
AC_DEFINE_UNQUOTED([PACKAGE_POINT_VERSION], $AX_POINT_VERSION, [Point part of version number])
AC_SUBST([PACKAGE_MAJOR_VERSION], $AX_MAJOR_VERSION)

AS_VAR_ARITH([current], $AX_MAJOR_VERSION+$AX_MINOR_VERSION)
AS_VAR_ARITH([revision], $AX_POINT_VERSION)
AS_VAR_ARITH([age], $AX_MINOR_VERSION)
AC_SUBST([LIBTOOL_CURRENT], $current)
AC_SUBST([LIBTOOL_REVISION], $revision)
AC_SUBST([LIBTOOL_AGE], $age)
AC_SUBST([LIBTOOL_VERSION_INFO], $current:$revision:$age)

AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
test "x$CXX" = "x" && AC_MSG_ERROR([unable to find C++ compiler])
AC_PROG_CC
test "x$CC" = "x" && AC_MSG_ERROR([unable to find C compiler])

ACX_PTHREAD(
    [ LIBS="$PTHREAD_LIBS $LIBS"
      CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
      CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
      CC="$PTHREAD_CC" ],
    [ AC_ERROR([No usable pthread library found]) ])

# Check libtool with modified compiler
AC_PROG_LIBTOOL

# Enable doxygen support
AX_PROG_DOXYGEN
DX_HTML_FEATURE(ON)
DX_PS_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_INIT_DOXYGEN(dStorm)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h windows.h winsock2.h math.h])
AC_CHECK_HEADERS([tiffio.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_CHECK_FUNCS([malloc floor sqrt strerror usleep])

# Checks for libraries.
AC_CHECK_LIB([tiff], [main], , [AC_ERROR(tiff library not found)])
AC_CHECK_LIB([boost_system], [main], , [AC_ERROR(boost system library not found)])
AC_CHECK_LIB([boost_thread], [main])
AC_CHECK_LIB([boost_thread_win32], [main])
AS_IF([test "x$ac_cv_lib_boost_thread_main" = "xno" -a "x$ac_cv_lib_boost_thread_win32_main" = "xno"],
    [AC_ERROR([No boost thread library found])], [])
AC_CHECK_LIB([ws2_32], [main])
AC_CHECK_LIB([mswsock], [main])

AC_ARG_WITH([windres],
    [AS_HELP_STRING([--with-windres],
        [path to windows resource compiler @<:@default=guess@:>@])],
        [],
        [ with_windres=guess ])
AS_IF( 
    [test "x$with_windres" = xguess], 
    [AC_CHECK_TOOL([WINDRES], [windres])],
    [])
AM_CONDITIONAL([HAVE_WINDRES], [test "x$WINDRES" != "x"])

AM_OPTIONS_WXCONFIG
reqwx=2.4.0
AM_PATH_WXCONFIG($reqwx, wxWin=1)
if test "$wxWin" != 1; then
    AC_MSG_ERROR([
      wxWidgets must be installed on your system.

      Please check that wx-config is in path, the directory
      where wxWidgets libraries are installed (returned by
      'wx-config --libs' or 'wx-config --static --libs' command)
      is in LD_LIBRARY_PATH or equivalent variable and
      wxWidgets version is $reqwx or above.
    ])
fi

CPPFLAGS="$CPPFLAGS $WX_CPPFLAGS"
CXXFLAGS="$CXXFLAGS $WX_CXXFLAGS_ONLY"
CFLAGS="$CFLAGS $WX_CFLAGS_ONLY"
LIBS="$LIBS $WX_LIBS"

PKG_CHECK_MODULES([DEPS], [librapidstorm GraphicsMagick++ any_iterator libsimparm])
CPPFLAGS="$CPPFLAGS $DEPS_CFLAGS"
LIBS="$DEPS_LIBS $LIBS"

PKG_CHECK_MODULES([librapidstormfile], [librapidstormfile])
PKG_CHECK_MODULES([libb64], [libb64])
PKG_CHECK_MODULES([locprec], [gsl])

AC_DEFINE([USE_GRAPHICSMAGICK], 1, [Use the GraphicsMagick library])

DSTORM_LIB_VERSION=`${PKG_CONFIG} --variable=identifier librapidstorm`
AC_SUBST([DSTORM_LIB_VERSION], $DSTORM_LIB_VERSION)
DSTORM_PLUGIN_DIR=`${PKG_CONFIG} --variable=plugindir librapidstorm`
AC_DEFINE_UNQUOTED([PLUGINDIR], "$DSTORM_PLUGIN_DIR", [Default directory for plugin files])
AC_SUBST([DSTORM_PLUGIN_DIR])

if echo "$LIBS" | grep "subsystem,windows"; then
    LIBS="$LIBS -Wl,--subsystem,console"
fi

# Integrate the help, library and the current src directory
CPPFLAGS="-I \$(top_builddir)/.. -I \$(top_srcdir) -I \$(top_srcdir)/.. $CPPFLAGS"

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
