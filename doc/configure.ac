AC_PREREQ(2.61)
AC_INIT([rapidSTORM], [3.1.0], [Steve Wolter <rapidstorm@swolter.sdf1.org>])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_AUX_DIR([ac-aux])

AM_INIT_AUTOMAKE
AM_MAINTAINER_MODE
AC_CONFIG_SRCDIR([rapidstorm.xml])

AX_SPLIT_VERSION
AC_SUBST( [PACKAGE_MAJOR_VERSION], $AX_MAJOR_VERSION )
AC_SUBST( [PACKAGE_MINOR_VERSION], $AX_MINOR_VERSION )
AC_SUBST( [PACKAGE_POINT_VERSION], $AX_POINT_VERSION )

AC_PROG_SED
test "x$SED" = "x" && AC_MSG_ERROR([unable to find stream editor])
AC_PROG_XSLTPROC
test "x$XSLTPROC" = "x" && AC_MSG_ERROR([unable to find XSLT processor])
AC_CHECK_PROGS([PDFXSLFO], [fop])
AM_CONDITIONAL([BUILD_PDF_DOCUMENTATION], [test -n "$PDFXSLFO"])

AC_CHECK_PROGS([ZIP], [zip], [ AC_MSG_ERROR([unable to find zip program]) ])
AC_CHECK_PROGS([BIB2XML], [bib2xml], [ AC_MSG_ERROR([unable to find bib2xml program from bibutils]) ])

build_date=`LC_ALL=C date "+%B %d, %Y"`
AC_SUBST([BUILD_DATE], "$build_date")
AC_SUBST([BUILD_YEAR], `date +%Y`)
AC_CONFIG_FILES([Makefile foprc version.ent])

AC_OUTPUT
