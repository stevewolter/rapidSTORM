noinst_LTLIBRARIES = libfreeoutputs.la libclosedoutputs.la

library_includedir = $(includedir)/dStorm/outputs
library_include_HEADERS = BinnedLocalizations.h LocalizationList.h TraceFilter.h

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    -I $(top_srcdir)/src

libfreeoutputs_la_SOURCES = \
    BinnedLocalizations.cpp LocalizationList.cpp TraceFilter.cpp 
libclosedoutputs_la_SOURCES = \
    AverageImage.cpp AverageImage.h BasicTransmissions.cpp \
    BinnedLocalizations_impl.h ColourDisplay.cpp \
    ColourDisplay.h ColourDisplay_impl.h \
    Histogram.h Histogram_impl.h \
    ImageDiscretizer.h ImageDiscretizer_impl.h \
    LocalizationCounter.cpp LocalizationCounter.h \
    LocalizationFile.cpp LocalizationFile.h \
    LocalizationFilter.cpp LocalizationFilter.h \
    ProgressMeter.cpp \
    ProgressMeter.h RawImageFile.cpp RawImageFile.h Slicer.cpp \
    Slicer.h \
    ViewportImage.h \
    PrecisionEstimator.h PrecisionEstimator.cpp \
    ViewerConfig.h Viewer.cpp Viewer.h \
    BasicTransmissions.h
