ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = debug.h Doxyfile 

dist_pkgdata_DATA = dstorm-config.txt

SUBDIRS = doc dStorm src test deploy

installer bundle :
	make -C deploy $@
