lib_LTLIBRARIES = libfitpp.la
libfitpp_la_SOURCES = Exponential2D.cpp LatticeFitFunction.cpp \
                       FitFunction.cpp 
library_includedir = $(includedir)/fit++
library_include_HEADERS = fit++/Exponential2D.hh fit++/FitFunction.hh fit++/LatticeFitFunction.hh
libfitpp_la_LDFLAGS = -no-undefined -version-info 1:0:0 

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CPPFLAGS += -Wall -Wextra \
    -include $(top_builddir)/config.h -include $(top_srcdir)/debug.h \
    -I $(srcdir)/fit++

