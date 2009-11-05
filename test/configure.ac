
AC_PREREQ(2.61)
AC_INIT([rapidStorm], [1.4.0], [dstorm at swolter dot sdf1 dot org])
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([../m4])

AC_LANG([C++])
AC_PROG_CXX
AC_PROG_LIBTOOL

AM_WITH_DMALLOC

# Integrate the help, library and the current src directory
CPPFLAGS="$CPPFLAGS -I \$(top_builddir)/../doc -I \$(top_srcdir)/../src -I \$(top_srcdir)/../lib"

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
