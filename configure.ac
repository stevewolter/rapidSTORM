#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([rapidStorm], [1.1], [dstorm at swolter dot sdf1 dot org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/dstorm.cpp])
AC_CONFIG_HEADER([config.h])

# Allow DLLs for Windows programs
AC_LIBTOOL_WIN32_DLL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL

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

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])
AC_CHECK_HEADER([Eigen/Array], [], [AC_ERROR([Missing eigen2 library])])

# Check whether to build against the Andor SDK
AC_CHECK_HEADERS([ATMCD32D.H], [build_adstorm=true])

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


LIBREADSIF_CPPFLAGS=
LIBREADSIF_LDFLAGS=
LIBREADSIF_LIBS=-lreadsif
if test "x$build_libreadsif" = "xtrue"; then
    LIBREADSIF_CPPFLAGS='-I$(top_srcdir)/readsif'
    LIBREADSIF_LDFLAGS='-L$(top_builddir)/readsif/.libs'
fi

AC_SUBST(LIBREADSIF_CPPFLAGS)
AC_SUBST(LIBREADSIF_LDFLAGS)
AC_SUBST(LIBREADSIF_LIBS)
AM_CONDITIONAL(BUILD_READSIF, [test x$build_libreadsif = xtrue])

# libfit++ for fitting
AC_ARG_WITH([libfit++], 
    [AS_HELP_STRING([--with-libfit++],
        [use system libfit++ instead of building own])],
    [],
    [build_libfitpp=true])

LIBLIBFITPP_CPPFLAGS=
LIBLIBFITPP_LDFLAGS=
LIBLIBFITPP_LIBS=-lfitpp
if test "x$build_libfitpp" = "xtrue"; then
    LIBLIBFITPP_CPPFLAGS='-I$(top_srcdir)/libfit++'
    LIBLIBFITPP_LDFLAGS='-L$(top_builddir)/libfit++/.libs'
fi

AC_SUBST(LIBLIBFITPP_CPPFLAGS)
AC_SUBST(LIBLIBFITPP_LDFLAGS)
AC_SUBST(LIBLIBFITPP_LIBS)
AM_CONDITIONAL(BUILD_LIBFITPP, [test x$build_libfitpp = xtrue])

# libdata-c++ for thread-safe data structures
AC_ARG_WITH([libdata-c++], 
    [AS_HELP_STRING([--with-libdata-c++],
        [use system libdata-c++ instead of building own])],
    [],
    [build_libdatapp=true])

LIBDATACPP_CPPFLAGS=
LIBDATACPP_LDFLAGS=
LIBDATACPP_LIBS=
if test "x$build_libdatapp" = "xtrue"; then
    LIBDATACPP_CPPFLAGS='-I$(top_srcdir)/libdata-c++'
fi

AC_SUBST(LIBDATACPP_CPPFLAGS)
AC_SUBST(LIBDATACPP_LDFLAGS)
AC_SUBST(LIBDATACPP_LIBS)
AM_CONDITIONAL(BUILD_DATACPP, [test x$build_libdatapp = xtrue])

# simparm for input/output of parameters
AC_ARG_WITH([simparm], 
    [AS_HELP_STRING([--with-simparm],
        [use system simparm instead of building own])],
    [],
    [build_simparm=true])

LIBSIMPARM_CPPFLAGS=
LIBSIMPARM_LDFLAGS=
LIBSIMPARM_LIBS=-lsimparm
if test "x$build_simparm" = "xtrue"; then
    LIBSIMPARM_CPPFLAGS='-I$(top_srcdir)/simparm/libsimparm'
    LIBSIMPARM_LDFLAGS='-L$(top_builddir)/simparm/libsimparm/.libs'
fi

AC_SUBST(LIBSIMPARM_CPPFLAGS)
AC_SUBST(LIBSIMPARM_LDFLAGS)
AC_SUBST(LIBSIMPARM_LIBS)
AM_CONDITIONAL(BUILD_SIMPARM, [test x$build_simparm = xtrue])

# Build the patched version of the cc++ lib
AC_ARG_WITH([patched-CommonC++], 
    [AS_HELP_STRING([--with-patched-CommonC++],
        [use system Common C++ library instead of building working
         version])],
    [],
    [build_commoncpp=true])

LIBCOMMONCPP_CPPFLAGS=
LIBCOMMONCPP_LDFLAGS=
if test "x$build_commoncpp" = "xtrue"; then
    LIBCOMMONCPP_CPPFLAGS='-I$(top_srcdir)/cc++'
    LIBCOMMONCPP_LDFLAGS='-L$(top_builddir)/cc++/.libs'
    LIBCOMMONCPP_LIBS=-lccgnu2_fixed
else
    LIBCOMMONCPP_LIBS=-lccgnu2
fi

AC_SUBST(LIBCOMMONCPP_CPPFLAGS)
AC_SUBST(LIBCOMMONCPP_LDFLAGS)
AC_SUBST(LIBCOMMONCPP_LIBS)
AM_CONDITIONAL(BUILD_COMMONCPP, [test x$build_commoncpp = xtrue])

# AndorCamera library

AC_ARG_WITH([AndorCamera], 
    [AS_HELP_STRING([--with-AndorCamera],
        [use system AndorCamera library])],
    [build_andorcam=false],
    [build_andorcam=true])

if test "x$build_andorcam" = "xtrue"; then
    LIBANDORCAMERA_CPPFLAGS='-I$(top_srcdir)/AndorCamera'
    LIBANDORCAMERA_LDFLAGS='-L$(top_builddir)/AndorCamera/.libs'
fi
if test "x$build_adstorm" = "xtrue"; then
    LIBANDORCAMERA_LIBS=-lAndorCamera
else
    LIBANDORCAMERA_LIBS=-lAndorCamConfig
fi

AC_SUBST(LIBANDORCAMERA_CPPFLAGS)
AC_SUBST(LIBANDORCAMERA_LDFLAGS)
AC_SUBST(LIBANDORCAMERA_LIBS)
AM_CONDITIONAL(BUILD_ADSTORM, [test x$build_adstorm = xtrue])

# CImgBuffer library

AC_ARG_WITH([CImgBuffer], 
    [AS_HELP_STRING([--with-CImgBuffer],
        [use system CImgBuffer library])],
    [],
    [build_cimgbuffer=true])

LIBCIMGBUFFER_CPPFLAGS=
LIBCIMGBUFFER_LDFLAGS=
LIBCIMGBUFFER_LIBS=-lCImgBuffer
if test "x$build_cimgbuffer" = "xtrue"; then
    LIBCIMGBUFFER_CPPFLAGS='-I$(top_srcdir)/CImgBuffer'
    LIBCIMGBUFFER_LDFLAGS='-L$(top_builddir)/CImgBuffer/.libs'
fi

AC_SUBST(LIBCIMGBUFFER_CPPFLAGS)
AC_SUBST(LIBCIMGBUFFER_LDFLAGS)
AC_SUBST(LIBCIMGBUFFER_LIBS)
AM_CONDITIONAL(BUILD_CIMGBUFFER, [test x$build_cimgbuffer = xtrue])

# dStorm library

AC_ARG_WITH([dStorm], 
    [AS_HELP_STRING([--with-dStorm],
        [use system dStorm library])],
    [],
    [build_dStorm=true])

LIBDSTORM_CPPFLAGS=
LIBDSTORM_LDFLAGS=
LIBDSTORM_LIBS=-ldStorm
if test "x$build_dStorm" = "xtrue"; then
    LIBDSTORM_CPPFLAGS='-I$(top_srcdir)/dStorm'
    LIBDSTORM_LDFLAGS='-L$(top_builddir)/dStorm/.libs'
fi

AC_SUBST(LIBDSTORM_CPPFLAGS)
AC_SUBST(LIBDSTORM_LDFLAGS)
AC_SUBST(LIBDSTORM_LIBS)
AM_CONDITIONAL(BUILD_DSTORM, [test x$build_dStorm = xtrue])

# locprec library

AC_ARG_ENABLE(locprec-use, 
    [AS_HELP_STRING([--enable-locprec],
        [enable use of locprec library])],
    [use_locprec=true],
    [use_locprec=false])

AC_ARG_WITH([locprec], 
    [AS_HELP_STRING([--with-locprec],
        [use system locprec library])],
    [],
    [build_locprec=true])

LIBLOCPREC_LDFLAGS=
if test "x$use_locprec" = "xtrue"; then
    LIBLOCPREC_CPPFLAGS=-DINCLUDE_LOCPREC
    LIBLOCPREC_LIBS=-llocprec
    if test "x$build_locprec" = "xtrue"; then
        LIBLOCPREC_CPPFLAGS+=' -I$(top_srcdir)/locprec'
        LIBLOCPREC_LDFLAGS='-L$(top_builddir)/locprec/.libs'
    fi
else
    LIBLOCPREC_CPPFLAGS=
    LIBLOCPREC_LIBS=
fi
if test "x$build_locprec" = "xtrue"; then
    AC_CHECK_LIB([gslcblas], [cblas_dsdot])
    AC_CHECK_LIB([gsl], [gsl_multifit_fdfsolver_set], , [AC_MSG_ERROR(GNU Scientific Library not found)])
fi

AC_SUBST(LIBLOCPREC_CPPFLAGS)
AC_SUBST(LIBLOCPREC_LDFLAGS)
AC_SUBST(LIBLOCPREC_LIBS)
AM_CONDITIONAL(USE_LOCPREC, [test x$use_locprec = xtrue])
AM_CONDITIONAL(BUILD_LOCPREC, [test x$use_locprec = xtrue -a x$build_locprec = xtrue])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

AM_WITH_DMALLOC

# Checks for library functions.
AC_CHECK_FUNCS([malloc floor sqrt strerror])

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
if test "x$build_locprec" = "xtrue"; then 
    AC_CONFIG_FILES([locprec/Makefile])
fi

AC_OUTPUT
