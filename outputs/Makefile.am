noinst_LTLIBRARIES = liboutputs.la

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_srcdir)/../lib -I $(top_builddir)/..

liboutputs_la_SOURCES = \
    AverageImage.cpp AverageImage.h BasicTransmissions.cpp \
    LocalizationCounter.cpp LocalizationCounter.h \
    LocalizationFile.cpp LocalizationFile.h \
    LocalizationFilter.cpp LocalizationFilter.h \
    ProgressMeter.cpp \
    ProgressMeter.h RawImageFile.cpp RawImageFile.h Slicer.cpp \
    Slicer.h \
    PrecisionEstimator.h PrecisionEstimator.cpp \
    BasicTransmissions.h
liboutputs_la_LIBADD = -lberghen_xmlParser
