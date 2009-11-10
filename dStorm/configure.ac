#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([rapidStorm], RAPIDSTORM_VERSION, RAPIDSTORM_MAINTAINER)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([output/Config.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([../m4])

AC_LANG([C++])

# Allow DLLs for Windows programs
AC_LIBTOOL_WIN32_DLL

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
DX_HTML_FEATURE(ON)
DX_PS_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_INIT_DOXYGEN(dStorm)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h windows.h])
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
# AC_CHECK_LIB([GraphicsMagick], [InitializeMagick])
# AC_CHECK_LIB([GraphicsMagick++], [main])

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

# Select the windowing library needed for CImg
case $host in 
   *mingw32*) LIBS="-luser32 -lgdi32 $LIBS" ;;
   *)         LIBS="$LIBS -lX11"            ;;
esac

CPPFLAGS="-I \$(top_builddir)/.. -I \$(top_srcdir)/.. $CPPFLAGS"

# Now write the global and doc/lib makefiles. More libs are needed for
# src.
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([helpers/Makefile ])
AC_CONFIG_FILES([data-c++/Makefile])
AC_CONFIG_FILES([input/Makefile engine/Makefile output/Makefile])
AC_CONFIG_FILES([wxDisplay/Makefile])
AC_CONFIG_FILES([outputs/Makefile])

AC_OUTPUT
