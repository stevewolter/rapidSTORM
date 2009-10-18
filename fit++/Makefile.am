check_PROGRAMS = alignment_test
TESTS = alignment_test

EXTRA_DIST = Exponential2D.hh LatticeFitFunction.hh \
                Exponential2D_impl.hh FitFunction.hh ResidueMatrix.hh \
		Position.hh Helpers.hh \
		BitfieldConstructor.hh BlockReturner.hh \
		Exponential2D_Correlated_Derivatives.hh \
		Exponential2D_ParameterHelper.hh \
		Exponential2D_Uncorrelated_Derivatives.hh \
		FitFunction_impl.hh

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CPPFLAGS += -Wall -Wextra \
    -include $(top_builddir)/config.h -include $(top_srcdir)/debug.h \
    -I $(srcdir)/fit++

alignment_test_LDFLAGS = -no-undefined -version-info 1:0:0 $(LIBCOMMONCPP_LDFLAGS)
alignment_test_CPPFLAGS = $(AM_CPPFLAGS) $(LIBCOMMONCPP_CPPFLAGS)
alignment_test_LDADD = $(LIBCOMMONCPP_LIBS) $(LIBCOMMONCPP_OBJECTS)
alignment_test_DEPENDENCIES = $(LIBCOMMONCPP_OBJECTS)
alignment_test_SOURCES = alignment_test.cpp
