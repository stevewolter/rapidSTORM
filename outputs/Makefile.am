noinst_LTLIBRARIES = liboutputs.la

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_srcdir)/../lib -I $(top_builddir)/..

liboutputs_la_SOURCES = \
    AverageImage.cpp AverageImage.h BasicTransmissions.cpp \
    ColourDisplay.cpp \
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
