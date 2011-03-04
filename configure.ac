#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([rapid2storm], [1.5.2], [Steve Wolter <rapidstorm@swolter.sdf1.org>])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])

DSTORM_LIB_VERSION=10
AC_SUBST([DSTORM_LIB_VERSION], $DSTORM_LIB_VERSION)

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])
AC_CONFIG_SRCDIR([dstorm.cpp])
AC_CONFIG_HEADERS([config.h])

AC_PROG_SED
test "x$SED" = "x" && AC_MSG_ERROR([unable to find stream editor])

AX_SPLIT_VERSION
AC_DEFINE_UNQUOTED([PACKAGE_MAJOR_VERSION], $AX_MAJOR_VERSION, [Major part of version number])
AC_DEFINE_UNQUOTED([PACKAGE_MINOR_VERSION], $AX_MINOR_VERSION, [Minor part of version number])
AC_DEFINE_UNQUOTED([PACKAGE_POINT_VERSION], $AX_POINT_VERSION, [Point part of version number])

AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
test "x$CXX" = "x" && AC_MSG_ERROR([unable to find C++ compiler])
AC_PROG_CC
test "x$CC" = "x" && AC_MSG_ERROR([unable to find C compiler])
AC_PROG_JAVAC
test "x$JAVAC" = "x" && AC_MSG_ERROR([unable to find Java compiler])
AC_PROG_JAR
test "x$JAR" = "x" && AC_MSG_ERROR([unable to find Java archiver])

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
AC_CHECK_HEADERS([stdlib.h string.h unistd.h windows.h math.h])
AC_CHECK_HEADER([Eigen/Array], [], [AC_ERROR([Missing eigen2 library])])
AC_CHECK_HEADERS([Magick++.h])
AC_CHECK_HEADERS([tiffio.h])
AC_CHECK_HEADERS([dStorm/Localization.h], [], [AC_ERROR([Missing dSTORM library])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_CHECK_FUNCS([malloc floor sqrt strerror usleep])

# Checks for libraries.
AC_CHECK_LIB([b64], [main], , [AC_ERROR(Base64 encoding library not found)])
AC_CHECK_LIB([dStorm], [main], , [AC_ERROR(dSTORM library not found)])
AC_CHECK_LIB([tiff], [main], , [AC_ERROR(tiff library not found)])
AC_CHECK_LIB([GraphicsMagick], [main], , [AC_ERROR(GraphicsMagick library not found)])
AC_CHECK_LIB([GraphicsMagick++], [main], , [AC_ERROR(GraphicsMagick++ library not found)])
AC_CHECK_LIB([readsif], [main], , [AC_ERROR(readsif library not found)])
AC_CHECK_LIB([boost_system], [main], , [AC_ERROR(boost system library not found)])
AC_CHECK_LIB([boost_thread], [main], , [AC_ERROR(boost thread library not found)])

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

AC_ARG_WITH([twiddler-jar],
    [AS_HELP_STRING([--with-twiddler-jar],
        [location of simparm twiddler.jar @<:@default=${prefix}/share/java/twiddler.jar@:>@])],
        [],
        [with_twiddler_jar=$prefix/share/java/twiddler.jar])

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

if echo "$LIBS" | grep "subsystem,windows"; then
    LIBS="$LIBS -Wl,--subsystem,console"
fi

AM_WITH_DMALLOC

# Integrate the help, library and the current src directory
CPPFLAGS="-I \$(top_builddir)/.. -I \$(top_srcdir) -I \$(top_srcdir)/.. $CPPFLAGS"

AC_SUBST([DSTORM_PLUGIN_DIR], "${libdir}/libdstorm/plugins-v${DSTORM_LIB_VERSION}")
AC_SUBST([TWIDDLER_JAR], $with_twiddler_jar)

AC_CONFIG_SUBDIRS([gauss_fitter 3d_fitter])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([manifest])

AC_OUTPUT
