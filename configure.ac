#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(RAPIDSTORM_NAME, RAPIDSTORM_VERSION,
        RAPIDSTORM_MAINTAINER)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])

CPPFLAGS="$CPPFLAGS -I $top_srcdir"

AC_CONFIG_SUBDIRS([doc])

AC_ARG_ENABLE([lib-build],
    [AS_HELP_STRING([--enable-lib-build], [Build library @<:@default=yes@:>@])],
    [],
    [enable_lib_build=yes])
if test "x$enable_lib_build" = "xyes"; then 
    AC_CONFIG_SUBDIRS([dStorm])
fi
AM_CONDITIONAL([BUILD_LIBRARY], [test "x$enable_lib_build" = "xyes"])
AC_ARG_ENABLE([exec-build],
    [AS_HELP_STRING([--enable-exec-build], [Build program executable @<:@default=yes@:>@])],
    [],
    [enable_exec_build=yes])
if test "x$enable_exec_build" = "xyes"; then 
    AC_CONFIG_SUBDIRS([src])
fi
AM_CONDITIONAL([BUILD_EXECUTABLE], [test "x$enable_exec_build" = "xyes"])
AC_ARG_ENABLE([unbundling-build],
    [AS_HELP_STRING([--enable-unbundling-build], [Unbundle lib/exec files from tar file])],
    [],
    [enable_unbundling_build=no])
if test "x$enable_unbundling_build" = "xyes"; then 
    AC_CONFIG_SUBDIRS([unbundle])
fi
AM_CONDITIONAL([UNBUNDLE], [test "x$enable_unbundling_build" = "xyes"])

AC_CONFIG_SUBDIRS([test deploy])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
