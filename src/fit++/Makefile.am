check_PROGRAMS = alignment_test
TESTS = alignment_test

library_includedir = $(includedir)/fit++
library_include_HEADERS = fit++/Exponential2D.hh fit++/FitFunction.hh fit++/LatticeFitFunction.hh \
                          fit++/Exponential2D_impl.hh fit++/FitFunction.hh fit++/ResidueMatrix.hh

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CPPFLAGS += -Wall -Wextra \
    -include $(top_builddir)/config.h -include $(top_srcdir)/debug.h \
    -I $(srcdir)/fit++

alignment_test_LDFLAGS = -no-undefined -version-info 1:0:0 $(LIBCOMMONCPP_LDFLAGS)
alignment_test_CPPFLAGS = $(AM_CPPFLAGS) $(LIBCOMMONCPP_CPPFLAGS)
alignment_test_LDADD = $(LIBCOMMONCPP_LIBS) $(LIBCOMMONCPP_OBJECTS)
alignment_test_DEPENDENCIES = $(LIBCOMMONCPP_OBJECTS)
alignment_test_SOURCES = alignment_test.cpp
