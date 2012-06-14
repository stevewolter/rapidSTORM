AC_PREREQ(2.61)
AC_INIT([rapidSTORM], [3.0.0], [Steve Wolter <rapidstorm@swolter.sdf1.org>])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([.])

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([dstorm.cpp])
AC_CONFIG_HEADERS([config.h])

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_SED
test "x$SED" = "x" && AC_MSG_ERROR([unable to find stream editor])

AX_SPLIT_VERSION
AC_SUBST( [PACKAGE_MAJOR_VERSION], $AX_MAJOR_VERSION )
AC_SUBST( [PACKAGE_MINOR_VERSION], $AX_MINOR_VERSION )
AC_SUBST( [PACKAGE_POINT_VERSION], $AX_POINT_VERSION )
# Substitute package version in the resource file
AC_DEFINE_UNQUOTED([PACKAGE_MAJOR_VERSION], $AX_MAJOR_VERSION, [Major component of package version])
AC_DEFINE_UNQUOTED([PACKAGE_MINOR_VERSION], $AX_MINOR_VERSION, [Minor component of package version])
AC_DEFINE_UNQUOTED([PACKAGE_POINT_VERSION], $AX_POINT_VERSION, [Point component of package version])
AC_SUBST([LIBTOOL_VERSION_INFO], 15:0:0)

AC_LANG([C++])

DX_HTML_FEATURE(ON)
DX_PDF_FEATURE(ON)
DX_INIT_DOXYGEN(rapidstorm, Doxyfile.in)

AC_LIBTOOL_WIN32_DLL

# Checks for programs.
AC_PROG_CXX
test "x$CXX" = "x" && AC_MSG_ERROR([unable to find C++ compiler])
AC_PROG_CC
test "x$CC" = "x" && AC_MSG_ERROR([unable to find C compiler])

AM_PROG_CC_C_O

# Check libtool with modified compiler
AC_PROG_LIBTOOL

PKG_CHECK_MODULES([base], [cs_units eigen3 any_iterator])
CPPFLAGS="$CPPFLAGS $base_CFLAGS"
LIBS="$base_LIBS $LIBS"

PKG_CHECK_MODULES([GraphicsMagick], [GraphicsMagick++])
PKG_CHECK_MODULES([gsl], [gsl])

tinyxml_has_pkgconfig=yes
PKG_CHECK_MODULES([tinyxml], [tinyxml], , [tinyxml_has_pkgconfig=no])
if test "$tinyxml_has_pkgconfig" = no; then
    tinyxml_CFLAGS=
    AC_CHECK_HEADER([tinyxml.h], [ true ], [ AC_ERROR([tinyxml.h header file not found]) ] )
    AC_CHECK_LIB([tinyxml], [main], [ tinyxml_LIBS="-ltinyxml" ], [ AC_ERROR([tinyxml library not found]) ] )
fi
AC_SUBST([tinyxml_LIBS])
AC_SUBST([tinyxml_CFLAGS])

have_libb64=yes
PKG_CHECK_MODULES([libb64], [libb64], , [have_libb64=no])
AS_IF([test "$have_libb64" = yes], AC_DEFINE([HAVE_LIBB64], [1], [LibB64 library is available]))

have_libreadsif=yes
PKG_CHECK_MODULES([libreadsif], [libreadsif], , [have_libreadsif=no])
AS_IF([test "$have_libreadsif" = yes], AC_DEFINE([HAVE_LIBREADSIF], [1], [readsif library is available]))
AM_CONDITIONAL([HAVE_LIBREADSIF], [test "$have_libreadsif" = yes])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h windows.h winsock2.h math.h sys/time.h sys/resource.h errno.h])
AC_CHECK_HEADER([boost/utility.hpp], [], [AC_ERROR([Missing boost headers])])
AC_CHECK_HEADERS([tiffio.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_CHECK_FUNCS([malloc floor sqrt strerror usleep getrusage])

# Checks for libraries.
AC_CHECK_LIB([boost_filesystem], [main], [boost_filesystem_LIB=-lboost_filesystem], [AC_ERROR(boost filesystem library not found)])
AC_CHECK_LIB([boost_system], [main], [boost_system_LIB=-lboost_system], [AC_ERROR(boost system library not found)])
AC_CHECK_LIB([boost_thread], [main], [boost_thread_LIB=-lboost_thread])
AC_CHECK_LIB([boost_thread_win32], [main], [boost_thread_LIB=-lboost_thread_win32])
AS_IF([test "x$ac_cv_lib_boost_thread_main" = "xno" -a "x$ac_cv_lib_boost_thread_win32_main" = "xno"],
    [AC_ERROR([No boost thread library found])], [])
AC_CHECK_LIB([boost_unit_test_framework], [main], [boost_unit_test_LIB=-lboost_unit_test_framework], [AC_ERROR(boost unit test library not found)])
AC_SUBST([boost_filesystem_LIB])
AC_SUBST([boost_system_LIB])
AC_SUBST([boost_thread_LIB])
AC_SUBST([boost_unit_test_LIB])

AC_CHECK_LIB([ws2_32], [main], [libb64_LIBS="$libb64_LIBS -lws2_32"])
AC_CHECK_LIB([mswsock], [main], [libb64_LIBS="$libb64_LIBS -lmswsock"])
AC_SUBST([libb64_LIBS], "$libb64_LIBS")

AC_CHECK_LIB([tiff], [main], [GraphicsMagick_LIBS="$GraphicsMagick_LIBS -ltiff"], [AC_ERROR(tiff library not found)])
AC_SUBST([GraphicsMagick_LIBS], "$GraphicsMagick_LIBS")

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

AC_ARG_ENABLE([specialized-fitters],
    [AS_HELP_STRING([--enable-specialized-fitters],
        [enable use of specialized fitters for common array sizes @<:@default=no@:>@])],
        [],
        [enable_specialized_fitters=no])
AS_IF(
    [test "x$enable_specialized_fitters" = xyes],
    [ AC_DEFINE([USE_SPECIALIZED_FITTERS], [1], [Use fitters specialized for common array sizes]) ],
    []
)
AM_CONDITIONAL([COMPILE_SPECIALIZED_FITTERS], [test "x$enable_specialized_fitters" = xyes])

AC_ARG_ENABLE([performance-tests], [AS_HELP_STRING([--enable-performance-tests],
    [run performance tests, whose results are host-dependent and normed on automatix])],
    [],
    [enable_performance_tests=no])
AC_ARG_WITH([telnet-host], [AS_HELP_STRING([--with-telnet-host], 
    [test on a remote host using telnet])],
    [],
    [with_telnet_host=])
AC_ARG_WITH([stage-dir], [AS_HELP_STRING([--with-setup-stage-dir], 
    [stage dir for remote testing])],
    [],
    [with_stage_dir=])
AC_ARG_ENABLE([camera-testing], [AS_HELP_STRING([--enable-camera-testing], 
    [run the camera connection test suite])], [],
    [enable_camera_testing=no])
AC_ARG_WITH([rapidstorm-test-cases], [AS_HELP_STRING([--with-rapidstorm-test-cases], 
    [directory with rapidSTORM test cases])],
    [],
    [with_rapidstorm_test_cases=])

AC_ARG_ENABLE([documentation], [AS_HELP_STRING([--enable-documentation],
    [build manual and doxygen documentation (default: yes)])],
    [],
    [enable_documentation=yes])
AM_CONDITIONAL([BUILD_DOCUMENTATION], [test "$enable_documentation" = 'yes'])

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

if echo "$WX_LIBS" | grep "subsystem,windows"; then
    WX_LIBS="$WX_LIBS -Wl,--subsystem,console"
fi

AC_DEFINE([USE_GRAPHICSMAGICK], 1, [Use the GraphicsMagick library])

if test "$enable_documentation" = 'yes'; then
    AC_CONFIG_SUBDIRS([doc])
fi

AC_SUBST([testcasedir], "$with_rapidstorm_test_cases")
AC_SUBST([setupstagedir], "$with_stage_dir")
AC_SUBST([TELNET_HOST], "$with_telnet_host")
AC_SUBST([test_performance], "$enable_performance_tests")
AC_SUBST([test_camera], "$enable_camera_testing")
AM_CONDITIONAL([REMOTE_TESTING], [test -n "$with_telnet_host"])

AC_CONFIG_FILES([Makefile rapidstorm.desktop install-dirs.exp])

AC_OUTPUT
