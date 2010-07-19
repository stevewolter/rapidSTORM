ACLOCAL_AMFLAGS = -I m4

EXTRA_DIST = debug.h Doxyfile 

dist_pkgdata_DATA = dstorm-config.txt

if BUILD_LIBRARY
DOC=doc
else
DOC=
endif

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

if BUILD_TESTS
TESTDIR=test
else
TESTDIR=
endif

if BUILD_DEPLOY
DEPLOYDIR=deploy
else
DEPLOYDIR=
endif

SUBDIRS = $(DOC) $(LIB_SUBDIR) $(EXEC_SUBDIR) $(TESTDIR) $(DEPLOYDIR)

installer bundle :
	test '$(DEPLOYDIR)' && make -C '$(DEPLOYDIR)' $@
