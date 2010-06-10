#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(RAPIDSTORM_NAME, RAPIDSTORM_VERSION, RAPIDSTORM_MAINTAINER)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])
AC_CONFIG_SRCDIR([dstorm.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([../m4])

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

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_CHECK_FUNCS([malloc floor sqrt strerror usleep])

# Checks for libraries.
AC_CHECK_LIB([simparm], [main])
AC_CHECK_LIB([tiff], [main])
AC_CHECK_LIB([GraphicsMagick], [main])
AC_CHECK_LIB([GraphicsMagick++], [main])
AC_CHECK_LIB([readsif], [main])

AC_CHECK_LIB([DummyAndorCamera], [main])
AS_IF([test "x$ac_cv_lib_DummyAndorCamera_main" = "xyes"],
    [ AC_MSG_NOTICE([enabling simulated Andor camera support]) ],
    [ AC_CHECK_LIB([ATMCD32D], [main]) ])
AS_IF([test "x$ac_cv_lib_ATMCD32D_main" = "xyes"],
    [ AC_MSG_NOTICE([enabling real Andor camera support]) ])
AM_CONDITIONAL([BUILD_ANDORCAM], [test "x$ac_cv_lib_DummyAndorCamera_main" = "xyes" -o "x$ac_cv_lib_ATMCD32D_main" = "xyes"])

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

AC_ARG_ENABLE([debug-plugin],
    [AS_HELP_STRING([--enable-debug-plugin],
        [enable building debug plugin @<:@default=yes@:>@])],
    [],
    [enable_debug_plugin=yes])
AM_CONDITIONAL( [BUILD_DEBUG_PLUGIN], [test "x$enable_debug_plugin" != xno] )
        
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

AC_SUBST([DSTORM_PLUGIN_DIR], "${libdir}/AC_PACKAGE_TARNAME/plugins")
AC_SUBST([TWIDDLER_JAR], $with_twiddler_jar)

AC_CONFIG_SUBDIRS([gauss_fitter])
AC_CONFIG_FILES([Makefile AndorCamera/Makefile fit++/Makefile])
AC_CONFIG_FILES([spotFinders/Makefile])
AC_CONFIG_FILES([engine/Makefile outputs/Makefile inputs/Makefile])
AC_CONFIG_FILES([wxDisplay/Makefile])
AC_CONFIG_FILES([viewer/Makefile])
AC_CONFIG_FILES([debug-plugin/Makefile])
AC_CONFIG_FILES([manifest])

AC_OUTPUT
