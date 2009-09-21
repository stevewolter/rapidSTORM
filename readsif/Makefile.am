include doxygen.am
EXTRA_DIST = Doxyfile
bin_PROGRAMS = sifInfo

lib_LTLIBRARIES = libreadsif.la
libreadsif_la_SOURCES = read_sif.c
libreadsif_la_LDFLAGS = -no-undefined -version-info 1:0:0
library_includedir = $(includedir)
library_include_HEADERS = read_sif.h

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CFLAGS = -Wall -Werror -Wextra

LDADD = -lreadsif
