lib_LTLIBRARIES = libdStorm.la
libdStorm_la_SOURCES = \
    SpotFitter.cpp SpotFitter.h \
    SpotFinder.cpp SpotFinder.h \
    Engine.cpp Engine.h \
    Config.cpp \
    GaussFitter.cpp GaussFitter.h \
    Localizations.cpp \
    SigmaGuesser.cpp SigmaGuesser.h  \
    Viewer.cpp Viewer.h \
    foreach.h \
    CandidateTree.cpp \
    Crankshaft.cpp \
    Carburettor.h \
    Car.cpp CarConfig.cpp \
    LocalizationFile.cpp LocalizationList.cpp \
    AverageImage.h AverageImage.cpp \
    GaussSmoothing.cpp GaussSmoothing.h \
    averageSmooth.cpp averageSmooth.h \
    dilation.h dilation.cpp \
    SigmaGuesser_FreeFormFitter.cpp \
    BinnedLocalizations.cpp Histogram.cpp \
    HighDepthImage.cpp \
    CImg_forward.h \
    ErosionSmoother.h \
    FillholeSmoother.cpp \
    FillholeSmoother.h \
    MedianSmoother.h \
    ParameterDisplay.h \
    ProgressMeter.h \
    Reconstruction.hh \
    Spalttiefpass.h \
    Variance.h \
    ViewerDisplay.h studentPinv.h studentPinv.c \
    Median.cpp \
    EmissionTracker.h EmissionTracker.cpp ObjectStore.h \
    TransmissionConfig.cpp BasicTransmissions.cpp \
    Trace.cpp TraceReducers.cpp
EXTRA_DIST = Reconstruction.cc

library_includedir = $(includedir)/dStorm
library_include_HEADERS = dStorm/Config.h dStorm/ShockAbsorberConfig.h \
    dStorm/ViewerConfig.h dStorm/Localization.h dStorm/Localizations.h \
    dStorm/Crankshaft.h dStorm/Transmission.h dStorm/CarConfig.h \
    dStorm/Car.h dStorm/BinnedLocalizations.h dStorm/LocalizationFile.h \
    dStorm/LocalizationList.h \
    dStorm/EngineDebug.h dStorm/LocalizationCounter.h \
    dStorm/HighDepthImage.h dStorm/Histogram.h dStorm/ViewportImage.h \
    dStorm/Candidate.h dStorm/CandidateTree.h dStorm/Image.h dStorm/Spot.h \
    dStorm/TransmissionConfig.h dStorm/BasicTransmissions.h \
    dStorm/Trace.h dStorm/TraceReducer.h

libdStorm_la_CPPFLAGS = -D_GNU_SOURCE -Wall -Wextra  \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    -I$(srcdir)/dStorm
libdStorm_la_LDFLAGS = -no-undefined -version-info 1:0:0
libdStorm_la_LIBADD = $(LIBOBJS)

libdStorm_la_LDFLAGS += $(LIBSIMPARM_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBSIMPARM_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBSIMPARM_LIBS)

libdStorm_la_LDFLAGS += $(LIBCOMMONCPP_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBCOMMONCPP_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBCOMMONCPP_LIBS)

libdStorm_la_LDFLAGS += $(LIBDATACPP_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBDATACPP_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBDATACPP_LIBS)

libdStorm_la_LDFLAGS += $(LIBCIMGBUFFER_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBCIMGBUFFER_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBCIMGBUFFER_LIBS)

libdStorm_la_LDFLAGS += $(LIBLIBFITPP_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBLIBFITPP_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBLIBFITPP_LIBS)

libdStorm_la_LIBADD += $(LIBCIMG_LIBS)
