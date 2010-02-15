ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign dejagnu

EXTRA_DIST = dSTORM.iss.in build_installer.mk.in
pkgdata_DATA = dSTORM.iss

installer bundle : build_installer.mk
	$(MAKE) -f build_installer.mk $@
