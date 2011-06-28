ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

bin_PROGRAMS = dstorm-@DSTORM_LIB_VERSION@
C_TESTS = check_resolutionsetter 
check_PROGRAMS = $(C_TESTS)
EXTRA_PROGRAMS = testDisplay 
noinst_LTLIBRARIES = \
    libinputs.la libnoopengine.la libengine.la liboutputs.la \
    libspotFinders.la libviewer.la libdStorm_display.la \
    libengine_stm.la 

install-exec-local :
	mkdir -p $(DESTDIR)$(bindir)
	$(RM) $(DESTDIR)$(bindir)/dstorm$(EXEEXT) && ln -s dstorm-@DSTORM_LIB_VERSION@$(EXEEXT) $(DESTDIR)$(bindir)/dstorm$(EXEEXT)

TESTS = $(C_TESTS)

doc_DATA = $(srcdir)/README $(srcdir)/LICENSE

if HAVE_WINDRES
    RESOURCE_FILE = windows_resources.rc
else
    RESOURCE_FILE =
endif

.rc.o : 
	$(WINDRES) -I . '$<' '$@'
windows_resources.o : config.h

libinputs_la_SOURCES = \
	inputs/AndorSIF.cpp inputs/TIFF.cpp \
	inputs/AndorSIF.h inputs/TIFF.h inputs/inputs.h inputs/inputs.cpp \
	inputs/TIFF_OpenFile.cpp \
	inputs/AndorSIF_OpenFile.h inputs/AndorSIF_OpenFile.cpp \
	inputs/BackgroundDeviationEstimator_decl.h \
	inputs/BackgroundDeviationEstimator.cpp \
	inputs/BackgroundDeviationEstimator.h \
	inputs/Splitter_decl.h inputs/Splitter.h inputs/Splitter.cpp \
	inputs/YMirror_decl.h inputs/YMirror.h inputs/YMirror.cpp \
	inputs/ResolutionSetter.h \
	inputs/ResolutionSetter_decl.h inputs/ResolutionSetter.h inputs/ResolutionSetter.cpp \
	inputs/SampleInfo.h inputs/SampleInfo.cpp \
    inputs/ROIFilter.h inputs/ROIFilter_decl.h inputs/ROIFilter.cpp \
    inputs/Basename_decl.h inputs/Basename.h inputs/Basename.cpp \
    inputs/Mask.h inputs/Mask.cpp \
    inputs/Buffer_decl.h inputs/Buffer.h inputs/Buffer_impl.h inputs/Buffer_impl_iterator.h \
    inputs/Buffer.cpp 

libnoopengine_la_SOURCES = \
	noop_engine/ChainLink.cpp noop_engine/ChainLink_decl.h \
	noop_engine/ChainLink.h noop_engine/Engine.cpp noop_engine/Engine.h
libengine_la_SOURCES = \
	engine/Config.cpp engine/Config_decl.h engine/Config.h \
	engine/Car_decl.h engine/Car.cpp engine/Car.h engine/Engine.cpp \
	    engine/EngineDebug.h engine/Engine.h \
	    engine/ImageFitter.h \
	    engine/SpotFinder.cpp engine/debug.h  \
	    engine/ChainLink_decl.h engine/ChainLink.h engine/ChainLink.cpp \
	    engine/PlaneFlattener.h engine/PlaneFlattener.cpp

libengine_stm_la_SOURCES = \
	    engine_stm/LocalizationBuncher.cpp engine_stm/LocalizationBuncher.h \
	    engine_stm/Config.h engine_stm/Config.cpp \
	    engine_stm/ChainLink_decl.h engine_stm/ChainLink.h engine_stm/ChainLink.cpp

liboutputs_la_SOURCES = \
    outputs/AverageImage.cpp outputs/AverageImage.h outputs/BasicTransmissions.cpp \
    outputs/LocalizationCounter.cpp outputs/LocalizationCounter.h \
    outputs/LocalizationFile.cpp outputs/LocalizationFile.h \
    outputs/ProgressMeter.cpp \
    outputs/ProgressMeter.h outputs/RawImageFile.cpp outputs/RawImageFile.h outputs/Slicer.cpp \
    outputs/Slicer.h \
    outputs/BasicTransmissions.h \
    outputs/MemoryCache.cpp outputs/MemoryCache.h outputs/MemoryCache_decl.h \
    outputs/MemoryCache_Cache.h outputs/MemoryCache_Cache.cpp
liboutputs_la_LIBADD = -lberghen_xmlParser

libspotFinders_la_SOURCES = \
    spotFinders/averageSmooth.cpp \
    spotFinders/averageSmooth.h \
    spotFinders/ErosionSmoother.h \
    spotFinders/ErosionSmoother.cpp \
    spotFinders/GaussSmoothing.cpp \
    spotFinders/GaussSmoothing.h \
    spotFinders/Median.cpp \
    spotFinders/MedianSmoother.h \
    spotFinders/MedianSmoother.cpp \
    spotFinders/Spalttiefpass.h \
    spotFinders/Spalttiefpass.cpp \
    spotFinders/spotFinders.cpp spotFinders/spotFinders.h

libviewer_la_SOURCES = \
    viewer/Display.h viewer/Display_inline.h viewer/Display_impl.h viewer/Display.cpp \
    viewer/ImageDiscretizer.h viewer/ImageDiscretizer_inline.h \
	viewer/ImageDiscretizer_impl.h viewer/ImageDiscretizer.cpp \
    viewer/Config_decl.h viewer/Config.h viewer/Config.cpp \
    viewer/Backend_decl.h viewer/Backend.h viewer/Backend.cpp \
    viewer/Viewer.cpp viewer/Viewer.h \
    viewer/Publisher.h viewer/HighDepth.h \
    viewer/LiveBackend_decl.h viewer/LiveBackend.h viewer/LiveBackend_impl.h viewer/LiveBackend.cpp \
    viewer/LiveCache.h viewer/LiveCache_inline.h viewer/LiveCache_impl.h viewer/LiveCache.cpp \
    viewer/TerminalBackend_decl.h viewer/TerminalBackend.h viewer/TerminalBackend_inline.h viewer/TerminalBackend_impl.h viewer/TerminalBackend.cpp \
    viewer/TerminalCache.h viewer/TerminalCache_inline.h viewer/TerminalCache_impl.h viewer/TerminalCache.cpp \
    viewer/BinnedLocalizations.cpp \
    viewer/Status_decl.h viewer/Status.h viewer/Status.cpp \
    viewer/LiveBackend_converter.h viewer/LiveBackend_converter.cpp \
    viewer/TerminalBackend_converter.h viewer/TerminalBackend_converter.cpp \
    viewer/ImageDiscretizer_converter.h \
    viewer/plugin.h \
    viewer/ColourScheme_decl.h viewer/ColourScheme.h  \
    viewer/colour_schemes/base.cpp viewer/colour_schemes/base_impl.h viewer/colour_schemes/base.h \
    viewer/colour_schemes/decl.h viewer/colour_schemes/impl.h viewer/colour_schemes/instantiate.h \
    viewer/colour_schemes/hot_config.cpp viewer/colour_schemes/hot_config.h viewer/colour_schemes/hot.h \
    viewer/colour_schemes/mono_config.cpp viewer/colour_schemes/mono_config.h viewer/colour_schemes/mono.h \
    viewer/colour_schemes/colored_config.cpp viewer/colour_schemes/colored_config.h viewer/colour_schemes/colored.h \
    viewer/colour_schemes/coordinate_config.cpp viewer/colour_schemes/coordinate_config.h viewer/colour_schemes/coordinate.h viewer/colour_schemes/coordinate.cpp \
    viewer/colour_schemes/HueSaturationMixer.h viewer/colour_schemes/HueSaturationMixer.cpp

libdStorm_display_la_SOURCES = \
    wxDisplay/wxManager.h wxDisplay/wxManager.cpp wxDisplay/App.h wxDisplay/App.cpp \
    wxDisplay/Window.h wxDisplay/Window.cpp wxDisplay/Canvas.h wxDisplay/Canvas.cpp \
    wxDisplay/ZoomSlider.h wxDisplay/ZoomSlider.cpp wxDisplay/Key.h wxDisplay/Key.cpp \
    wxDisplay/helpers.h wxDisplay/ScaleBar.h wxDisplay/ScaleBar.cpp wxDisplay/helpers.cpp \
    wxDisplay/wxManager_store_image.cpp wxDisplay/SizeConvert.h

testDisplay_SOURCES = wxDisplay/test.cpp
testDisplay_LDADD = libdStorm_display.la
check_resolutionsetter_SOURCES = inputs/ResolutionSetter_check.cpp
check_resolutionsetter_LDADD = libinputs.la

dstorm_@DSTORM_LIB_VERSION@_SOURCES = \
    dstorm.cpp \
    CommandLine.h CommandLine.cpp \
    ModuleLoader.h ModuleLoader.cpp \
    LibraryHandle.h LibraryHandle.cpp \
    JobStarter.h JobStarter.cpp \
    InputStream.h InputStream.cpp \
    debug.h debug_print.h $(RESOURCE_FILE) \
    TIFFOperation.h TIFFOperation.cpp
dstorm_@DSTORM_LIB_VERSION@_LDADD = liboutputs.la libviewer.la \
    libspotFinders.la \
    libengine_stm.la \
    libengine.la libnoopengine.la \
    libinputs.la \
    libdStorm_display.la \
    -lltdl 

