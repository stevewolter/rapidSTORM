
AC_PREREQ(2.61)
AC_INIT(RAPIDSTORM_NAME, RAPIDSTORM_VERSION, RAPIDSTORM_MAINTAINER)

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE

AC_CONFIG_MACRO_DIR([../m4])

# Check for ISCC Inno Setup help compiler
AC_CHECK_PROGS([ISCC], [iscc])
test "x$ISCC" != "x" || AC_ERROR([Inno Setup compiler not found])
AC_CHECK_PROGS([WINE], [wine])
AC_CHECK_PROGS([PATH_REWRITER], ["cygpath --windows" echo])

AC_ARG_WITH([additional-install-file],
    [AS_HELP_STRING([--with-additional-install-file],
        [additional local installer file in ISS format]) ] )
AC_SUBST([EXTRA_INSTALLER], [$with_additional_install_file])

AC_CONFIG_FILES([Makefile dSTORM.iss build_installer.mk])
AC_OUTPUT
