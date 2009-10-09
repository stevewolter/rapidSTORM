ACLOCAL_AMFLAGS = -I m4

include doxygen.am

EXTRA_DIST = debug.h doxygen.am Doxyfile

SUBDIRS = src test 

dist-jar : all
	make -C src dist-jar
