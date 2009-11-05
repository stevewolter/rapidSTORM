
noinst_LTLIBRARIES = libspotFinders.la

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/.. -I $(top_srcdir)/../lib

libspotFinders_la_SOURCES = \
    averageSmooth.cpp \
    averageSmooth.h \
    ErosionSmoother.h \
    GaussSmoothing.cpp \
    GaussSmoothing.h \
    Median.cpp \
    MedianSmoother.h \
    Spalttiefpass.h \
    spotFinders.cpp spotFinders.h
