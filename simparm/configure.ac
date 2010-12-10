#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([simparm], [0.4], [simparm at swolter dot sdf1 dot org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([libsimparm/ConfigSet.cc])
# AC_CONFIG_HEADER([config.h])

# Allow DLLs for Windows programs
AC_LIBTOOL_WIN32_DLL

# Checks for programs.
AC_PROG_CXX
AC_CHECK_CLASSPATH
AC_PROG_JAVAC
AC_PROG_JAR
AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])
AC_LANG_CPLUSPLUS

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([pthread.h])

AC_CONFIG_FILES([Makefile libsimparm/Makefile twiddler/Makefile multiplex/Makefile])
AC_OUTPUT
