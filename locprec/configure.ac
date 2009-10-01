#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([locprec], [1.0.0], [dstorm at swolter dot sdf1 dot org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([NoiseSource.cpp])
AC_CONFIG_HEADER([config.h])

# Allow DLLs for Windows programs
AC_LIBTOOL_WIN32_DLL

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
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

AC_LANG_CPLUSPLUS
 
# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h windows.h])
AC_CHECK_HEADER([Eigen/Array], [], [AC_ERROR([Missing eigen2 library])])

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
    LIBDATACPP_CPPFLAGS='-I$(top_srcdir)/../libdata-c++'
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
    LIBSIMPARM_CPPFLAGS='-I$(top_srcdir)/../simparm/libsimparm'
    LIBSIMPARM_OBJECTS='$(top_builddir)/../simparm/libsimparm/libsimparm.la'
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
    LIBCOMMONCPP_CPPFLAGS='-I$(top_srcdir)/../cc++'
    LIBCOMMONCPP_OBJECTS='$(top_builddir)/../cc++/libccgnu2_fixed.la'
else
    LIBCOMMONCPP_LIBS=-lccgnu2
fi

AC_SUBST(LIBCOMMONCPP_CPPFLAGS)
AC_SUBST(LIBCOMMONCPP_LDFLAGS)
AC_SUBST(LIBCOMMONCPP_LIBS)
AC_SUBST(LIBCOMMONCPP_OBJECTS)
AM_CONDITIONAL(BUILD_COMMONCPP, [test x$build_commoncpp = xtrue])

# dStorm library

# AC_ARG_WITH([dStorm], 
    # [AS_HELP_STRING([--with-dStorm],
        # [use system dStorm library])],
    # [],
    # [build_dStorm=true])
# 
# if test "x$build_dStorm" = "xtrue"; then
    # LIBDSTORM_CPPFLAGS='-I$(top_srcdir)/../dStorm'
    # LIBDSTORM_OBJECTS='$(top_builddir)/../dStorm/libdStorm.la'
# else
    # LIBDSTORM_LIBS=-ldStorm
# fi
# 
# AC_SUBST(LIBDSTORM_CPPFLAGS)
# AC_SUBST(LIBDSTORM_LDFLAGS)
# AC_SUBST(LIBDSTORM_LIBS)
# AC_SUBST(LIBDSTORM_OBJECTS)
# AM_CONDITIONAL(BUILD_DSTORM, [test x$build_dStorm = xtrue])

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

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

# Modify libtool to use -lstdc++_s instead of -lstdc++ for Mingw32.
# This is necessary for correct handling of cross-DLL exceptions.
if test "x$MINGW32_CXX" = "xtrue"; then
    sed -e 's/-lstdc++ /-lstdc++_s /' libtool > libtool.tmp
    mv libtool.tmp libtool
fi
