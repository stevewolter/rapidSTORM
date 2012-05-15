#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([simparm], [5.6.0], [simparm at swolter dot sdf1 dot org])
AC_CONFIG_SRCDIR([libsimparm/ConfigSet.cc])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([../m4])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE

# Allow DLLs for Windows programs
AC_LIBTOOL_WIN32_DLL

AC_PROG_SED
test "x$SED" = "x" && AC_MSG_ERROR([unable to find stream editor])

AC_LANG_CPLUSPLUS

# Checks for programs.
AC_PROG_CXX
AC_CHECK_CLASSPATH
AC_PROG_JAVAC
AC_CHECK_PROGS([JAR], [jar jar$EXEEXT])
AS_IF([test "x$JAR" = "x"], [AC_ERROR([No usable java archiver found])])

# Checks for libraries.
ACX_PTHREAD(
    [ LIBS="$PTHREAD_LIBS $LIBS"
      CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
      CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
      CC="$PTHREAD_CC" ],
    [ AC_ERROR([No usable pthread library found]) ])

# Check for libtool after compiler was set
AC_PROG_LIBTOOL
LT_PROG_RC
AM_CONDITIONAL([HAVE_WINDRES], test "x$RC" != "x")

PKG_CHECK_MODULES([eigen], [eigen3])
CXXFLAGS="$CXXFLAGS $eigen_CFLAGS"
LIBS="$eigen_LIBS $LIBS"

# Checks for header files.
AC_HEADER_STDC

AC_CONFIG_FILES([Makefile libsimparm/Makefile twiddler/Makefile multiplex/Makefile])
AC_CONFIG_FILES([twiddler/twiddler])
AC_OUTPUT

