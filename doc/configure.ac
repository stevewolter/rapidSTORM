AC_PREREQ(2.61)
AC_INIT([rapidSTORM], [2.22.0], [Steve Wolter <rapidstorm@swolter.sdf1.org>])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([ac-aux])

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([rapidstorm.xml])

AC_PROG_SED
test "x$SED" = "x" && AC_MSG_ERROR([unable to find stream editor])
AC_PROG_XSLTPROC
test "x$XSLTPROC" = "x" && AC_MSG_ERROR([unable to find XSLT processor])
AC_CHECK_PROGS([PDFXSLFO], [fop])
AM_CONDITIONAL([BUILD_PDF_DOCUMENTATION], [test -n "$PDFXSLFO"])

AC_CHECK_PROGS([ZIP], [zip], [ AC_MSG_ERROR([unable to find zip program]) ])
AC_CHECK_PROGS([BIB2XML], [bib2xml], [ AC_MSG_ERROR([unable to find bib2xml program from bibutils]) ])

AC_CONFIG_FILES([Makefile foprc])

AC_OUTPUT
