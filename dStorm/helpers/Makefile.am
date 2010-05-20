noinst_LTLIBRARIES = libhelpers.la
libhelpers_la_SOURCES = thread.cpp DisplayManager.cpp dilation.cpp \
    BlockingThreadRegistry.cpp

library_includedir = $(includedir)/dStorm/helpers
library_include_HEADERS = \
    thread.h Runnables.h dilation.h Variance.h \
    DisplayDataSource.h DisplayDataSource_impl.h \
    DisplayManager.h errors.h MayBeASignal.h \
    BlockingThreadRegistry.h

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CPPFLAGS += -Wall -Wextra \
    -include $(top_srcdir)/../debug.h
