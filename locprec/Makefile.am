noinst_PROGRAMS = fitNoise
lib_LTLIBRARIES = liblocprec.la
liblocprec_la_SOURCES = \
    RegionSegmenter.cpp \
    foreach.h RegionSegmenterConfig.cpp \
    NoiseSource.cpp \
    NoiseMeter.h Fluorophore.cpp \
    pixelatedBessel.h pixelatedBessel.cpp \
    SpotFinderEstimator.h SpotFinderEstimator.cpp \
    NoiseGenerator.cpp Counter.h Noise.h SpotMeter.h \
    MaximumSearcher.cpp PrecisionEstimator.cpp

#   DriftEstimator.h DriftEstimator.cpp
#   PrecisionMeter.h PrecisionMeter.cpp

library_include_HEADERS = \
    locprec/RegionSegmenterConfig.h \
    locprec/RegionSegmenter.h locprec/NoiseSource.h\
    locprec/NoiseGenerator.h locprec/Fluorophore.h locprec/relate.h \
    locprec/MaximumSearcher.h
library_includedir = $(includedir)/locprec

LDADD_BASIC =
LDFLAGS_BASIC = 
CPPFLAGS_BASIC = -D_POSIX_SOURCE -Wall -Wextra \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    -I$(srcdir)/locprec -I$(top_srcdir)/lib

LDFLAGS_BASIC += $(LIBDSTORM_LDFLAGS)
CPPFLAGS_BASIC += $(LIBDSTORM_CPPFLAGS)
LDADD_BASIC += $(LIBDSTORM_LIBS)

LDFLAGS_BASIC += $(LIBSIMPARM_LDFLAGS)
CPPFLAGS_BASIC += $(LIBSIMPARM_CPPFLAGS)
LDADD_BASIC += $(LIBSIMPARM_LIBS)

LDFLAGS_BASIC += $(LIBCOMMONCPP_LDFLAGS)
CPPFLAGS_BASIC += $(LIBCOMMONCPP_CPPFLAGS)
LDADD_BASIC += $(LIBCOMMONCPP_LIBS)

LDFLAGS_BASIC += $(LIBCIMGBUFFER_LDFLAGS)
CPPFLAGS_BASIC += $(LIBCIMGBUFFER_CPPFLAGS)
LDADD_BASIC += $(LIBCIMGBUFFER_LIBS)

LDFLAGS_BASIC += $(LIBLIBFITPP_LDFLAGS)
CPPFLAGS_BASIC += $(LIBLIBFITPP_CPPFLAGS)
LDADD_BASIC += $(LIBLIBFITPP_LIBS)

LDFLAGS_BASIC += $(LIBDATACPP_LDFLAGS)
CPPFLAGS_BASIC += $(LIBDATACPP_CPPFLAGS)
LDADD_BASIC += $(LIBDATACPP_LIBS)

LDADD_BASIC += $(LIBCIMG_LIBS)

AM_CPPFLAGS = -D_POSIX_SOURCE $(CPPFLAGS_BASIC)
AM_LDFLAGS = $(LDFLAGS_BASIC) 
LDADD = $(LDADD_BASIC)

liblocprec_la_LDFLAGS = $(AM_LDFLAGS) -no-undefined -version-info 1:0:0
liblocprec_la_LIBADD = $(LIBOBJS) $(LDADD)

fitNoise_SOURCES = fitNoise.cpp
fitNoise_LDADD = $(LDADD) -lgsl -lgslcblas
