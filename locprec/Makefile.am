noinst_PROGRAMS = fitNoise
lib_LTLIBRARIES = liblocprec.la
liblocprec_la_SOURCES = \
    foreach.h TransmissionSourceFactory.cpp \
    NoiseSource.cpp \
    NoiseMeter.h Fluorophore.cpp \
    pixelatedBessel.h pixelatedBessel.cpp \
    SpotFinderEstimator.h SpotFinderEstimator.cpp \
    NoiseGenerator.cpp Counter.h Noise.h SpotMeter.h \
    PrecisionEstimator.cpp RegionSegmenter.cpp \
    EmissionTracker.h EmissionTracker.cpp \
    SpotMeter.cpp NoiseMeter.cpp \
    FluorophoreDistributions_Random.cpp \
    FluorophoreDistributions_Lattice.cpp \
    FluorophoreDistributions_Lines.cpp \
    plugin.cpp

#   DriftEstimator.h DriftEstimator.cpp
#   PrecisionMeter.h PrecisionMeter.cpp

library_include_HEADERS = \
    locprec/TransmissionSourceFactory.h \
    locprec/RegionSegmenter.h locprec/NoiseSource.h\
    locprec/NoiseGenerator.h locprec/Fluorophore.h locprec/relate.h
library_includedir = $(includedir)/locprec

LDADD_BASIC =
LDFLAGS_BASIC = 
CPPFLAGS_BASIC = -D_POSIX_SOURCE -Wall -Wextra \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    -I$(srcdir)/locprec -I$(top_srcdir)/lib
LIBRARY_OBJECTS =

LDFLAGS_BASIC += $(LIBSIMPARM_LDFLAGS)
CPPFLAGS_BASIC += $(LIBSIMPARM_CPPFLAGS)
LDADD_BASIC += $(LIBSIMPARM_LIBS)
LIBRARY_OBJECTS += $(LIBSIMPARM_OBJECTS)

LDFLAGS_BASIC += $(LIBCOMMONCPP_LDFLAGS)
CPPFLAGS_BASIC += $(LIBCOMMONCPP_CPPFLAGS)
LDADD_BASIC += $(LIBCOMMONCPP_LIBS)
LIBRARY_OBJECTS += $(LIBCOMMONCPP_OBJECTS)

LDFLAGS_BASIC += $(LIBLIBFITPP_LDFLAGS)
CPPFLAGS_BASIC += $(LIBLIBFITPP_CPPFLAGS)
LDADD_BASIC += $(LIBLIBFITPP_LIBS)
LIBRARY_OBJECTS += $(LIBLIBFITPP_OBJECTS)

LDFLAGS_BASIC += $(LIBDATACPP_LDFLAGS)
CPPFLAGS_BASIC += $(LIBDATACPP_CPPFLAGS)
LDADD_BASIC += $(LIBDATACPP_LIBS)
LIBRARY_OBJECTS += $(LIBDATACPP_OBJECTS)

LDADD_BASIC += $(LIBCIMG_LIBS)
LIBRARY_OBJECTS += $(LIBCIMG_OBJECTS)

AM_CPPFLAGS = -D_POSIX_SOURCE $(CPPFLAGS_BASIC)
AM_LDFLAGS = $(LDFLAGS_BASIC) 
LDADD = $(LDADD_BASIC) $(LIBRARY_OBJECTS)

liblocprec_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined -version-info 1:0:0
liblocprec_la_LIBADD = $(LIBOBJS) $(LDADD)
liblocprec_la_DEPENDENCIES = $(LIBRARY_OBJECTS)

fitNoise_SOURCES = fitNoise.cpp
fitNoise_LDADD = $(LDADD) -lgsl -lgslcblas
