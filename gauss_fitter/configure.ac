#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([rapidstorm_fitters], [1.0], RAPIDSTORM_MAINTAINER)
AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE([enable])
AC_CONFIG_SRCDIR([GaussFitter.cpp])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([../../m4])

AC_PROG_SED
test "x$SED" = "x" && AC_MSG_ERROR([unable to find stream editor])

AX_SPLIT_VERSION
AC_DEFINE_UNQUOTED([PACKAGE_MAJOR_VERSION], $AX_MAJOR_VERSION, [Major part of version number])
AC_DEFINE_UNQUOTED([PACKAGE_MINOR_VERSION], $AX_MINOR_VERSION, [Minor part of version number])
AC_DEFINE_UNQUOTED([PACKAGE_POINT_VERSION], $AX_POINT_VERSION, [Point part of version number])

AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
test "x$CXX" = "x" && AC_MSG_ERROR([unable to find C++ compiler])
AC_PROG_CC
test "x$CC" = "x" && AC_MSG_ERROR([unable to find C compiler])

# Check libtool with modified compiler
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADER([Eigen/Array], [], [AC_ERROR([Missing eigen2 library])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_STRUCT_TM

# Checks for library functions.
AC_CHECK_FUNCS([sqrt])

# Checks for libraries.
AC_CHECK_LIB([simparm], [main])

AC_ARG_ENABLE([specialized-fitters],
    [AS_HELP_STRING([--enable-specialized-fitters],
        [enable use of specialized fitters for common array sizes @<:@default=on@:>@])],
        [],
        [enable_specialized_fitters=yes])
AS_IF(
    [test "x$enable_specialized_fitters" = xyes],
    [ AC_DEFINE([USE_SPECIALIZED_FITTERS], [1], [Use fitters specialized for common array sizes]) ],
    []
)

# Integrate the help, library and the current src directory
CPPFLAGS="-I \$(top_builddir)/../.. -I \$(top_srcdir) -I \$(top_srcdir)/.. -I \$(top_srcdir)/../.. $CPPFLAGS"

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
