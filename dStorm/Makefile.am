lib_LTLIBRARIES = libdStorm.la
libdStorm_la_SOURCES = \
    engine/SpotFitter.cpp engine/GaussFitter_selector.cpp \
    engine/GaussFitter_Width_Invariants.cpp \
    engine/GaussFitter_Specialized_a.cpp \
    engine/GaussFitter_Specialized_b.cpp \
    engine/GaussFitter_Specialized_c.cpp \
    engine/GaussFitter_Specialized_FreeSigma.cpp  \
    engine/Carburettor.cpp \
    engine/CandidateTree.cpp engine/CarConfig.cpp \
    engine/Car.cpp engine/Config.cpp engine/Engine.cpp \
    engine/Engine.h engine/foreach.h engine/GaussFitter.cpp \
    engine/GaussFitter.h engine/Localizations.cpp \
    engine/SigmaFitter.h engine/SigmaGuesser.cpp \
    engine/SigmaGuesser_FreeFormFitter.cpp engine/SigmaGuesser.h \
    engine/Trace.cpp engine/TraceReducers.cpp \
    engine/TransmissionConfig.cpp engine/BaseTransmissionConfigs.cpp \
    engine/LocalizationFileReader.cpp engine/LocalizationBuncher.cpp \
    engine/LocalizationBuncher.h \
    spotFinders/averageSmooth.cpp \
    spotFinders/averageSmooth.h spotFinders/dilation.cpp \
    spotFinders/dilation.h spotFinders/ErosionSmoother.h \
    spotFinders/FillholeSmoother.cpp spotFinders/FillholeSmoother.h \
    spotFinders/GaussSmoothing.cpp spotFinders/GaussSmoothing.h \
    spotFinders/Median.cpp spotFinders/MedianSmoother.h \
    spotFinders/Reconstruction.cc spotFinders/Reconstruction.hh \
    spotFinders/Spalttiefpass.h spotFinders/SpotFinder.cpp \
    statistics/studentPinv.c transmissions/AverageImage.cpp \
    transmissions/BasicTransmissions.cpp  \
    transmissions/BinnedLocalizations.cpp \
    transmissions/ColourDisplay.cpp \
    transmissions/Crankshaft.cpp \
    transmissions/HighDepthImage.cpp \
    transmissions/LocalizationFile.cpp \
    transmissions/LocalizationFilter.cpp \
    transmissions/LocalizationList.cpp \
    transmissions/Viewer.cpp transmissions/TraceFilter.cpp \
    transmissions/ProgressMeter.cpp \
    transmissions/LocalizationCounter.cpp \
    transmissions/Slicer.cpp  \
    transmissions/RawImageFile.cpp
EXTRA_DIST = spotFinders/Reconstruction.cc

library_includedir = $(includedir)/dStorm
library_include_HEADERS = dStorm/BasicOutputs.h \
    dStorm/engine/Candidate.h dStorm/engine/CandidateTree.h \
    dStorm/engine/Input.h \
    dStorm/CarConfig.h \
    dStorm/Car.h dStorm/engine/CImg_forward.h \
    dStorm/engine/Config.h dStorm/engine/EngineDebug.h \
    dStorm/engine/ImageFitter.h dStorm/engine/Image.h \
    dStorm/engine/Localization.h dStorm/engine/Localizations.h \
    dStorm/engine/SpotFinder.h dStorm/engine/SpotFitter.h \
    dStorm/engine/Spot.h dStorm/engine/Trace.h \
    dStorm/engine/TraceReducer.h dStorm/OutputSource.h \
    dStorm/OutputBuilder.h dStorm/OutputFactory.h \
    dStorm/FilterSource.h dStorm/FilterBuilder.h \
    dStorm/engine/LocalizationFileReader.h \
    dStorm/Output.h dStorm/statistics/studentPinv.h \
    dStorm/statistics/Variance.h dStorm/transmissions/AverageImage.h \
    dStorm/transmissions/BinnedLocalizations.h \
    dStorm/transmissions/BinnedLocalizations_impl.h \
    dStorm/transmissions/Crankshaft.h \
    dStorm/transmissions/HighDepthImage.h \
    dStorm/transmissions/HighDepthImage_impl.h \
    dStorm/transmissions/Histogram.h \
    dStorm/transmissions/LocalizationCounter.h \
    dStorm/transmissions/LocalizationFile.h \
    dStorm/transmissions/LocalizationList.h \
    dStorm/transmissions/ProgressMeter.h \
    dStorm/transmissions/ViewerConfig.h \
    dStorm/transmissions/Viewer.h \
    dStorm/transmissions/ViewportImage.h \
    dStorm/transmissions/Slicer.h \
    dStorm/transmissions/RawImageFile.h

libdStorm_la_CPPFLAGS = -D_GNU_SOURCE -Wall -Wextra  \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    -I$(srcdir)/dStorm \
    --param inline-unit-growth=1000
libdStorm_la_LDFLAGS = -no-undefined -version-info 1:0:0
libdStorm_la_LIBADD = $(LIBOBJS)
libdStorm_la_DEPENDENCIES = 

libdStorm_la_LDFLAGS += $(LIBCOMMONCPP_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBCOMMONCPP_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBCOMMONCPP_LIBS) $(LIBCOMMONCPP_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBCOMMONCPP_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBDATACPP_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBDATACPP_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBDATACPP_LIBS) $(LIBDATACPP_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBDATACPP_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBCIMGBUFFER_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBCIMGBUFFER_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBCIMGBUFFER_LIBS) $(LIBCIMGBUFFER_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBCIMGBUFFER_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBLIBFITPP_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBLIBFITPP_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBLIBFITPP_LIBS) $(LIBLIBFITPP_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBLIBFITPP_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBSIMPARM_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBSIMPARM_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBSIMPARM_LIBS) $(LIBSIMPARM_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBSIMPARM_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBCIMG_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBCIMG_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBCIMG_LIBS) $(LIBCIMG_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBCIMG_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBMAGICK_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBMAGICK_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBMAGICK_LIBS) $(LIBMAGICK_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBMAGICK_OBJECTS)

if BUILD_ADSTORM
    libdStorm_la_LDFLAGS += $(LIBANDORCAMERA_LDFLAGS)
    libdStorm_la_CPPFLAGS += $(LIBANDORCAMERA_CPPFLAGS)
    libdStorm_la_LIBADD += $(LIBANDORCAMERA_LIBS) \
                               $(LIBANDORCAMERA_OBJECTS)
    libdStorm_la_DEPENDENCIES += $(LIBANDORCAMERA_OBJECTS)
endif

if BUILD_STATIC_PROGRAMS
    libdStorm_la_LDFLAGS += -static
endif

