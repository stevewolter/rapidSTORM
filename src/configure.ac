#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([rapidStorm], RAPIDSTORM_VERSION, RAPIDSTORM_MAINTAINER)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([dstorm.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([../m4])

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
AC_CHECK_LIB([GraphicsMagick++], [main])
AC_CHECK_LIB([readsif], [main])

AC_ARG_WITH([andor-camera],
    [AS_HELP_STRING([--with-andor-camera],
        [support direct acquisition from Andor cameras by Andor SDK @<:@default=check@:>@])],
        [],
        [with_andor_camera=check])
AS_IF( 
    [test "x$with_andor_camera" != xno], 
        [AC_CHECK_LIB([ATMCD32D], [main], ,
            [with_andor_camera=no])] )
AM_CONDITIONAL( [HAVE_LIBATMCD32D], [test "x$with_andor_camera" != xno] )

AC_ARG_WITH([twiddler-jar],
    [AS_HELP_STRING([--with-twiddler-jar],
        [location of simparm twiddler.jar @<:@default=${prefix}/share/java/twiddler.jar@:>@])],
        [],
        [with_twiddler_jar=$prefix/share/java/twiddler.jar])

AC_ARG_WITH([specialized-fitters],
    [AS_HELP_STRING([--with-specialized-fitters],
        [enable use of specialized fitters for common array sizes @<:@default=on@:>@])],
        [],
        [with_specialized_fitters=yes])
AS_IF(
    [test "x$with_specialized_fitters" = xyes],
    [ AC_DEFINE([USE_SPECIALIZED_FITTERS], [1], [Use fitters specialized for common array sizes]) ],
    []
)
 
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

AM_WITH_DMALLOC

# Select the windowing library needed for CImg
case $host in 
   *mingw32*) LIBCIMG_LIBS='-luser32 -lgdi32 ' ;;
   *)         LIBCIMG_LIBS=-lX11            ;;
esac

# Integrate the help, library and the current src directory
CPPFLAGS="-I \$(top_builddir)/.. -I \$(top_srcdir) -I \$(top_srcdir)/.. $CPPFLAGS"

AC_SUBST([DSTORM_PLUGIN_DIR], "${libdir}/AC_PACKAGE_NAME/plugins")
AC_SUBST([TWIDDLER_JAR], $with_twiddler_jar)

AC_CONFIG_FILES([Makefile AndorCamera/Makefile fit++/Makefile])
AC_CONFIG_FILES([spotFinders/Makefile])
AC_CONFIG_FILES([engine/Makefile outputs/Makefile inputs/Makefile])
AC_CONFIG_FILES([wxDisplay/Makefile])
AC_CONFIG_FILES([viewer/Makefile])
AC_CONFIG_FILES([manifest])

AC_OUTPUT
