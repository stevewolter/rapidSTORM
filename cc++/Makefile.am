lib_LTLIBRARIES = libccgnu2_fixed.la
libccgnu2_fixed_la_SOURCES = thread.cpp 
library_includedir = $(includedir)/cc++
library_include_HEADERS = cc++/thread.h
libccgnu2_fixed_la_LDFLAGS = -no-undefined -version-info 1:0:0 

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CPPFLAGS += -Wall -Wextra \
    -include $(top_builddir)/config.h -include $(top_srcdir)/debug.h

if BUILD_STATIC_PROGRAMS
    libccgnu2_fixed_la_LDFLAGS += -static
endif
