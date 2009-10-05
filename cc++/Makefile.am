noinst_LTLIBRARIES = libccgnu2_fixed.la
libccgnu2_fixed_la_SOURCES = thread.cpp 

library_includedir = $(includedir)/dStorm/helpers
library_include_HEADERS = thread.h

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CPPFLAGS += -Wall -Wextra \
    -include $(top_builddir)/config.h -include $(top_srcdir)/debug.h
