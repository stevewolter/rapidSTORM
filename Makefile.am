ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = debug.h Doxyfile 

dist_pkgdata_DATA = dstorm-config.txt

if BUILD_LIBRARY
LIB_SUBDIR=dStorm
else
LIB_SUBDIR=
endif

if BUILD_EXECUTABLE
EXEC_SUBDIR=src
else
EXEC_SUBDIR=
endif

if UNBUNDLE
UNBUNDLE_SUBDIR=unbundle
else
UNBUNDLE_SUBDIR=
endif

SUBDIRS = doc $(LIB_SUBDIR) $(EXEC_SUBDIR) $(UNBUNDLE_SUBDIR) test deploy

installer bundle :
	make -C deploy $@
