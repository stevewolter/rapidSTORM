noinst_LTLIBRARIES = libhelpers.la
libhelpers_la_SOURCES = thread.cpp DisplayManager.cpp dilation.cpp

library_includedir = $(includedir)/dStorm/helpers
library_include_HEADERS = \
    thread.h Runnables.h dilation.h Variance.h \
    DisplayDataSource.h DisplayManager.h

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CPPFLAGS += -Wall -Wextra \
    -include $(top_srcdir)/../debug.h
