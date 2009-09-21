#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([readsif], [1.0], [readsif at swolter dot sdf1 dot org])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([read_sif.c])
AC_CONFIG_HEADERS([config.h])

# Allow DLLs for Windows programs
AC_LIBTOOL_WIN32_DLL

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL

# Enable doxygen support
AX_PROG_DOXYGEN
DX_HTML_FEATURE(ON)
DX_INIT_DOXYGEN(libreadsif)

# Checks for header files.
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
