#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([rapi.dStorm-doc], RAPIDSTORM_VERSION, RAPIDSTORM_MAINTAINER)
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([../m4])

AC_PROG_XSLTPROC
test "x$XSLTPROC" = "x" && AC_MSG_ERROR([unable to find XSLT processor])
AC_PROG_HHC
test "x$HHC" = "x" && AC_MSG_ERROR([unable to find MS HTML Help compiler])
AC_PROG_LN_S

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([help/Makefile])

AC_OUTPUT
