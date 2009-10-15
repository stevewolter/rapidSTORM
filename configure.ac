#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([rapidStorm], [1.4.0], [dstorm at swolter dot sdf1 dot org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/dstorm.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

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
AC_PROG_XSLTPROC
test "x$XSLTPROC" = "x" && AC_MSG_ERROR([unable to find XSLT processor])
AC_PROG_HHC
test "x$HHC" = "x" && AC_MSG_ERROR([unable to find MS HTML Help compiler])

# Mingw32 needs -mthreads for correct multi-threading behaviour
# and -shared-libgcc for correct cross-DLL exception handling
case $host in
    *mingw32*) 
        CC="$CC -shared-libgcc"
        CPPFLAGS="$CPPFLAGS -D_GLIBCXX_DLL"
        CFLAGS="$CFLAGS -mthreads"
        CXX="$CXX -shared-libgcc"
        CXXFLAGS="$CXXFLAGS -mthreads"
    ;;
esac

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
AC_CHECK_LIB([pthread], [pthread_create], , [AC_MSG_ERROR(POSIX thread library not found)])
AC_CHECK_LIB([simparm], [main])
AC_CHECK_LIB([GraphicsMagick], [InitializeMagick])
AC_CHECK_LIB([GraphicsMagick++], [main])
AC_CHECK_LIB([readsif], [main])
AC_LANG_CPLUSPLUS
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

AC_ARG_WITH([andor-camera],
    [AS_HELP_STRING([--with-andor-camera],
        [support direct acquisition from Andor cameras by Andor SDK @<:@default=check@:>@])],
        [],
        [with_andor_camera=check])
AS_IF( 
    [test "x$with_andor_camera" != xno], 
        [AC_CHECK_LIB([ATMCD32D], [main], , [with_andor_camera=no])] )
AM_CONDITIONAL( [HAVE_LIBATMCD32D], [test "x$with_andor_camera" != xno] )
 
AM_WITH_DMALLOC

# Select the windowing library needed for CImg
case $host in 
   *mingw32*) LIBCIMG_LIBS='-luser32 -lgdi32 ' ;;
   *)         LIBCIMG_LIBS=-lX11            ;;
esac

AC_SUBST([DSTORM_PLUGIN_DIR], "${libdir}/AC_PACKAGE_NAME/plugins")

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([doc/help/Makefile])
AC_CONFIG_FILES([src/Makefile src/AndorCamera/Makefile])
AC_CONFIG_FILES([src/data-c++/Makefile])
AC_CONFIG_FILES([src/cc++/Makefile src/fit++/Makefile])
AC_CONFIG_FILES([src/CImgBuffer/Makefile src/dStorm/Makefile])
AC_CONFIG_FILES([src/engine/Makefile src/outputs/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([src/manifest])

AC_OUTPUT
