#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(RAPIDSTORM_NAME, RAPIDSTORM_VERSION,
        RAPIDSTORM_MAINTAINER)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])

CPPFLAGS="$CPPFLAGS -I $top_srcdir"

AC_ARG_ENABLE([doc-build],
    [AS_HELP_STRING([--enable-doc-build], [Build documentation @<:@default=yes@:>@])],
    [],
    [enable_doc_build=yes])
if test "x$enable_doc_build" = "xyes"; then 
    AC_CONFIG_SUBDIRS([doc])
fi
AM_CONDITIONAL([BUILD_DOC], [test "x$enable_doc_build" = "xyes"])

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

AC_ARG_ENABLE([test-build],
    [AS_HELP_STRING([--enable-test-build], [Build tests @<:@default=yes@:>@])],
    [],
    [enable_test_build=yes])
if test "x$enable_test_build" = "xyes"; then 
    AC_CONFIG_SUBDIRS([test])
fi
AM_CONDITIONAL([BUILD_TESTS], [test "x$enable_test_build" = "xyes"])

AC_ARG_ENABLE([deploy-build],
    [AS_HELP_STRING([--enable-deploy-build], [Build remote deployment files @<:@default=yes@:>@])],
    [],
    [enable_deploy_build=yes])
if test "x$enable_deploy_build" = "xyes"; then 
    AC_CONFIG_SUBDIRS([deploy])
fi
AM_CONDITIONAL([BUILD_DEPLOY], [test "x$enable_deploy_build" = "xyes"])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
