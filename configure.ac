#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([rapidStorm], [1.3.0], [dstorm at swolter dot sdf1 dot org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/dstorm.cpp])
AC_CONFIG_HEADER([config.h])

# LT_INIT([win32-dll])

# Allow DLLs for Windows programs
AC_LIBTOOL_WIN32_DLL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_JAVAC
AC_PROG_JAR
AC_PROG_LIBTOOL

# Test whether C/C++ compilers are mingw32's.
if "$CC" --version 2>/dev/null | grep mingw32; then
    MINGW32_CC="true"
else
    MINGW32_CC="false"
fi
if "$CXX" --version 2>/dev/null | grep mingw32; then
    MINGW32_CXX="true"
else
    MINGW32_CXX="false"
fi
    
# Mingw32 needs -mthreads for correct multi-threading behaviour
# and -shared-libgcc for correct cross-DLL exception handling
if test "x$MINGW32_CC" = "xtrue"; then
    CC="$CC -shared-libgcc"
    CFLAGS="$CFLAGS -mthreads"
fi
if test "x$MINGW32_CXX" = "xtrue"; then
    CXX="$CXX -shared-libgcc"
    CXXFLAGS="$CXXFLAGS -mthreads"
fi

# Enable doxygen support
AX_PROG_DOXYGEN
DX_HTML_FEATURE(ON)
DX_PS_FEATURE(OFF)
DX_PDF_FEATURE(OFF)
DX_INIT_DOXYGEN(dStorm)

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create], , [AC_MSG_ERROR(POSIX thread library not found)])
AC_CHECK_LIB([Magick++], [InitializeMagick])
AC_LANG_CPLUSPLUS
# AC_CHECK_LIB([simparm], [main])
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

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h windows.h])
AC_CHECK_HEADER([Eigen/Array], [], [AC_ERROR([Missing eigen2 library])])
AC_CHECK_HEADERS([tiffio.h])

# Andor camera simulation for testing
AC_ARG_WITH([simulated-camera],
    [AS_HELP_STRING([--with-simulated-camera],
        [enable camera simulation])],
    [use_camsim=true build_camsim=true]
)
AC_ARG_WITH([andor-header],
    [AS_HELP_STRING([--with-andor-header],
        [supply full path to ATMCD32D.H here])],
    [build_adstorm=true andor_header=$withval],
    [build_adstorm=false]
)

AH_TEMPLATE([HAVE_ANDOR_CAMERA], 
    [Will be set to 1 when the Andor SDK is present])

ANDOR_SDK_CPPFLAGS=
ANDOR_SDK_LDFLAGS=
ANDOR_SDK_LIBS=
ANDOR_SDK_OBJECTS=
if test "x$use_camsim" = "xtrue"; then
    build_adstorm=true
    build_camsim=true
    ANDOR_SDK_CPPFLAGS='-I$(top_srcdir)/DummyCamera'
    ANDOR_SDK_OBJECTS='$(top_builddir)/DummyCamera/libdummy_andor_camera.la'
else
    if test "x$build_adstorm" = "xtrue"; then
        ANDOR_SDK_LIBS=-lATMCD32D
    fi
fi
# AM_CONDITIONAL(MODIFY_ANDOR_HEADER, [test "x$andor_header" != "x"])
AM_CONDITIONAL(MODIFY_ANDOR_HEADER, [false])
AC_SUBST(ANDOR_HEADER, [$andor_header])

AM_CONDITIONAL(BUILD_CAMSIM, [test x$build_camsim = xtrue])
AC_SUBST(ANDOR_SDK_CPPFLAGS)
AC_SUBST(ANDOR_SDK_LDFLAGS)
AC_SUBST(ANDOR_SDK_LIBS)
AC_SUBST(ANDOR_SDK_OBJECTS)

if test "x$build_adstorm" = "xtrue"; then
    AC_DEFINE([HAVE_ANDOR_CAMERA])
fi

LIBMAGICK_LDFLAGS="-lGraphicsMagick -lGraphicsMagick++"
AC_SUBST(LIBMAGICK_CPPFLAGS)
AC_SUBST(LIBMAGICK_LDFLAGS)
AC_SUBST(LIBMAGICK_LIBS)
AC_SUBST(LIBMAGICK_OBJECTS)

#### Now check with libraries we should build ourselves. ###

# libreadsif for reading Andor SIF files
AC_ARG_WITH([libreadsif], 
    [AS_HELP_STRING([--with-libreadsif],
        [use system libreadsif instead of building own])],
    [],
    [build_libreadsif=true])

AH_TEMPLATE([D3_INTERNAL_VERSION], [])
AC_ARG_ENABLE([D3-internal],
    [AS_HELP_STRING([--D3-internal])],
    [AC_DEFINE(D3_INTERNAL_VERSION)], [])
AC_ARG_ENABLE([all-static-build],
    [AS_HELP_STRING([--enable-all-static-build])],
    [build_all_static=true], [])
AM_CONDITIONAL(BUILD_STATIC_PROGRAMS, [test "x$build_all_static" = xtrue])

LIBREADSIF_CPPFLAGS=
LIBREADSIF_LDFLAGS=
LIBREADSIF_LIBS=
if test "x$build_libreadsif" = "xtrue"; then
    LIBREADSIF_CPPFLAGS='-I$(top_srcdir)/readsif'
    LIBREADSIF_OBJECTS='$(top_builddir)/readsif/libreadsif.la'
else
    LIBREADSIF_LIBS=-lreadsif
fi

AC_SUBST(LIBREADSIF_CPPFLAGS)
AC_SUBST(LIBREADSIF_LDFLAGS)
AC_SUBST(LIBREADSIF_LIBS)
AC_SUBST(LIBREADSIF_OBJECTS)
AM_CONDITIONAL(BUILD_READSIF, [test x$build_libreadsif = xtrue])

# libfit++ for fitting
AC_ARG_WITH([libfit++], 
    [AS_HELP_STRING([--with-libfit++],
        [use system libfit++ instead of building own])],
    [],
    [build_libfitpp=true])

if test "x$build_libfitpp" = "xtrue"; then
    LIBLIBFITPP_CPPFLAGS='-I$(top_srcdir)/libfit++'
fi

AC_SUBST(LIBLIBFITPP_CPPFLAGS)
AC_SUBST(LIBLIBFITPP_LDFLAGS)
AC_SUBST(LIBLIBFITPP_LIBS)
AC_SUBST(LIBLIBFITPP_OBJECTS)
AM_CONDITIONAL(BUILD_LIBFITPP, [test x$build_libfitpp = xtrue])

# libdata-c++ for thread-safe data structures
AC_ARG_WITH([libdata-c++], 
    [AS_HELP_STRING([--with-libdata-c++],
        [use system libdata-c++ instead of building own])],
    [],
    [build_libdatapp=true])

if test "x$build_libdatapp" = "xtrue"; then
    LIBDATACPP_CPPFLAGS='-I$(top_srcdir)/libdata-c++'
fi

AC_SUBST(LIBDATACPP_CPPFLAGS)
AC_SUBST(LIBDATACPP_LDFLAGS)
AC_SUBST(LIBDATACPP_LIBS)
AC_SUBST(LIBDATACPP_OBJECTS)
AM_CONDITIONAL(BUILD_DATACPP, [test x$build_libdatapp = xtrue])

# simparm for input/output of parameters
AC_ARG_WITH([simparm], 
    [AS_HELP_STRING([--with-simparm],
        [use system simparm instead of building own])],
    [],
    [build_simparm=true])

if test "x$build_simparm" = "xtrue"; then
    LIBSIMPARM_CPPFLAGS='-I$(top_srcdir)/simparm/libsimparm'
    LIBSIMPARM_OBJECTS='$(top_builddir)/simparm/libsimparm/libsimparm.la'
else
    LIBSIMPARM_LIBS=-lsimparm
fi

AC_SUBST(LIBSIMPARM_CPPFLAGS)
AC_SUBST(LIBSIMPARM_LDFLAGS)
AC_SUBST(LIBSIMPARM_LIBS)
AC_SUBST(LIBSIMPARM_OBJECTS)
AM_CONDITIONAL(BUILD_SIMPARM, [test x$build_simparm = xtrue])

# Build the patched version of the cc++ lib
AC_ARG_WITH([patched-CommonC++], 
    [AS_HELP_STRING([--with-patched-CommonC++],
        [use system Common C++ library instead of building working
         version])],
    [],
    [build_commoncpp=true])

if test "x$build_commoncpp" = "xtrue"; then
    LIBCOMMONCPP_CPPFLAGS='-I$(top_srcdir)/cc++'
    LIBCOMMONCPP_OBJECTS='$(top_builddir)/cc++/libccgnu2_fixed.la'
else
    LIBCOMMONCPP_LIBS=-lccgnu2
fi

AC_SUBST(LIBCOMMONCPP_CPPFLAGS)
AC_SUBST(LIBCOMMONCPP_LDFLAGS)
AC_SUBST(LIBCOMMONCPP_LIBS)
AC_SUBST(LIBCOMMONCPP_OBJECTS)
AM_CONDITIONAL(BUILD_COMMONCPP, [test x$build_commoncpp = xtrue])

# AndorCamera library

AC_ARG_WITH([AndorCamera], 
    [AS_HELP_STRING([--with-AndorCamera],
        [use system AndorCamera library])],
    [build_andorcam=false],
    [build_andorcam=true])

if test "x$build_andorcam" = "xtrue"; then
    LIBANDORCAMERA_CPPFLAGS='-I$(top_srcdir)/AndorCamera'
    if test "x$build_adstorm" = "xtrue"; then
        LIBANDORCAMERA_OBJECTS='$(top_builddir)/AndorCamera/libAndorCamera.la'
    else
        LIBANDORCAMERA_OBJECTS='$(top_builddir)/AndorCamera/libAndorCamConfig.la'
    fi
else
    if test "x$build_adstorm" = "xtrue"; then
        LIBANDORCAMERA_LIBS=-lAndorCamera
    else
        LIBANDORCAMERA_LIBS=-lAndorCamConfig
    fi
fi

AC_SUBST(LIBANDORCAMERA_CPPFLAGS)
AC_SUBST(LIBANDORCAMERA_LDFLAGS)
AC_SUBST(LIBANDORCAMERA_LIBS)
AC_SUBST(LIBANDORCAMERA_OBJECTS)
AM_CONDITIONAL(BUILD_ADSTORM, [test x$build_adstorm = xtrue])

# CImgBuffer library

AC_ARG_WITH([CImgBuffer], 
    [AS_HELP_STRING([--with-CImgBuffer],
        [use system CImgBuffer library])],
    [],
    [build_cimgbuffer=true])

if test "x$build_cimgbuffer" = "xtrue"; then
    LIBCIMGBUFFER_CPPFLAGS='-I$(top_srcdir)/CImgBuffer'
    LIBCIMGBUFFER_OBJECTS='$(top_builddir)/CImgBuffer/libCImgBuffer.la'
else
    LIBCIMGBUFFER_LIBS=-lCImgBuffer
fi

AC_SUBST(LIBCIMGBUFFER_CPPFLAGS)
AC_SUBST(LIBCIMGBUFFER_LDFLAGS)
AC_SUBST(LIBCIMGBUFFER_LIBS)
AC_SUBST(LIBCIMGBUFFER_OBJECTS)
AM_CONDITIONAL(BUILD_CIMGBUFFER, [test x$build_cimgbuffer = xtrue])

# dStorm library

AC_ARG_WITH([dStorm], 
    [AS_HELP_STRING([--with-dStorm],
        [use system dStorm library])],
    [],
    [build_dStorm=true])

if test "x$build_dStorm" = "xtrue"; then
    LIBDSTORM_CPPFLAGS='-I$(top_srcdir)/dStorm'
    LIBDSTORM_OBJECTS='$(top_builddir)/dStorm/libdStorm.la'
else
    LIBDSTORM_LIBS=-ldStorm
fi

AC_SUBST(LIBDSTORM_CPPFLAGS)
AC_SUBST(LIBDSTORM_LDFLAGS)
AC_SUBST(LIBDSTORM_LIBS)
AC_SUBST(LIBDSTORM_OBJECTS)
AM_CONDITIONAL(BUILD_DSTORM, [test x$build_dStorm = xtrue])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

AM_WITH_DMALLOC

# Checks for library functions.
AC_CHECK_FUNCS([malloc floor sqrt strerror usleep])

# Select the windowing library needed for CImg
case $host in 
   *mingw32*) LIBCIMG_LIBS='-luser32 -lgdi32 ' ;;
   *)         LIBCIMG_LIBS=-lX11            ;;
esac
AC_SUBST([LIBCIMG_LIBS])

AC_CONFIG_FILES([Makefile src/Makefile test/Makefile])
AC_CONFIG_FILES([AndorCamera/Makefile])
if test "x$build_libreadsif" = "xtrue"; then 
    AC_CONFIG_SUBDIRS([readsif])
fi
if test "x$build_libfitpp" = "xtrue"; then 
    AC_CONFIG_FILES([libfit++/Makefile])
fi
if test "x$build_libdatapp" = "xtrue"; then 
    AC_CONFIG_FILES([libdata-c++/Makefile])
fi
if test "x$build_simparm" = "xtrue"; then 
    AC_CONFIG_SUBDIRS([simparm])
fi
if test "x$build_commoncpp" = "xtrue"; then 
    AC_CONFIG_FILES([cc++/Makefile])
fi
if test "x$build_cimgbuffer" = "xtrue"; then 
    AC_CONFIG_FILES([CImgBuffer/Makefile])
fi
if test "x$build_dStorm" = "xtrue"; then 
    AC_CONFIG_FILES([dStorm/Makefile])
fi
if test "x$build_camsim" = "xtrue"; then 
    AC_CONFIG_FILES([DummyCamera/Makefile])
fi
AC_CONFIG_SUBDIRS([locprec])
AC_CONFIG_FILES([src/manifest])

AC_OUTPUT

# Modify libtool to use -lstdc++_s instead of -lstdc++ for Mingw32.
# This is necessary for correct handling of cross-DLL exceptions.
if test "x$MINGW32_CXX" = "xtrue"; then
    sed -e 's/-lstdc++ /-lstdc++_s /' libtool > libtool.tmp
    mv libtool.tmp libtool
fi
