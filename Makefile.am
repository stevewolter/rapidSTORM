ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

plugin_libdir = $(DSTORM_PLUGIN_DIR)
javadir=$(datadir)/java
javabuilddir = classes
applicationsdir=$(datadir)/applications

C_TESTS = check_resolutionsetter checkplugin_locprec reconstruction checkplugin_camera checkplugin_testplugin

bin_PROGRAMS = dstorm-@DSTORM_LIB_VERSION@ 
bin_SCRIPTS = rapidstorm@DSTORM_LIB_VERSION@
check_PROGRAMS = $(C_TESTS)
noinst_LTLIBRARIES = \
    libinputs.la libnoopengine.la libengine.la liboutputs.la \
    libspotFinders.la libviewer.la libdStorm_display.la \
    libengine_stm.la 
plugin_lib_LTLIBRARIES = liblocprec.la libdirectcamerainput.la libtestplugin.la
if HAVE_JAVA_TOOLCHAIN
java_DATA = rapidSTORM@DSTORM_LIB_VERSION@.jar 
applications_DATA=rapidstorm@DSTORM_LIB_VERSION@.desktop
endif
doc_DATA = $(srcdir)/README 
pkgdata_DATA = 

JAVASOURCES = DStorm.java DefaultPath.java
JARCONTENT = logo.png

EXTRA_PROGRAMS = gradient determine-clusters-from-centers fit-camera-noise testDisplay 
EXTRA_DIST = DStorm.java $(JARCONTENT) 
CLEANFILES = rapidSTORM@DSTORM_LIB_VERSION@.jar \
	rapidstorm@DSTORM_LIB_VERSION@.desktop rapidstorm@DSTORM_LIB_VERSION@

rapidSTORM@DSTORM_LIB_VERSION@.jar : $(JAVASOURCES) manifest $(TWIDDLER_JAR) $(JARCONTENT)
	$(MKDIR_P) $(javabuilddir)
	$(JAVAC) -classpath $(CLASSPATH):$(TWIDDLER_JAR): -d $(javabuilddir) $(filter %.java,$^)
	cp $(TWIDDLER_JAR) $@
	$(JAR) -uMf $@ -C $(srcdir) $(JARCONTENT) -C $(javabuilddir) .
	$(JAR) -umf manifest $@

rapidstorm@DSTORM_LIB_VERSION@.desktop : rapidstorm.desktop
	cp $< $@

rapidstorm@DSTORM_LIB_VERSION@ : rapidstorm
	cp $< $@

clean-local :
	-rm -fr $(javabuilddir)

TESTS = $(C_TESTS)


if HAVE_WINDRES
    RESOURCE_FILE = windows_resources.rc
else
    RESOURCE_FILE =
endif

.rc.o : 
	$(WINDRES) -I . '$<' '$@'
windows_resources.o : config.h

libinputs_la_SOURCES = \
	inputs/TIFF.cpp \
	inputs/TIFF.h inputs/inputs.h inputs/inputs.cpp \
	inputs/TIFF_OpenFile.cpp \
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
	engine/Engine.cpp \
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
    outputs/ProgressMeter.cpp \
    outputs/ProgressMeter.h outputs/RawImageFile.cpp outputs/RawImageFile.h outputs/Slicer.cpp \
    outputs/Slicer.h \
    outputs/BasicTransmissions.h \
    outputs/MemoryCache.cpp outputs/MemoryCache.h outputs/MemoryCache_decl.h \
    outputs/MemoryCache_Cache.h outputs/MemoryCache_Cache.cpp

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

liblocprec_la_LDFLAGS = -no-undefined -version-info $(LIBTOOL_VERSION_INFO) -module
liblocprec_la_LIBADD = $(locprec_LIBS)
liblocprec_la_CPPFLAGS = $(locprec_CFLAGS)
liblocprec_la_SOURCES = \
    locprec/plugin.cpp locprec/foreach.h locprec/NoiseSource.cpp \
    locprec/NoiseMeter.h locprec/Fluorophore.cpp \
    locprec/pixelatedBessel.h locprec/pixelatedBessel.cpp \
    locprec/NoiseGenerator.cpp locprec/Counter.h locprec/Noise.h locprec/SpotMeter.h \
    locprec/RegionSegmenter.cpp \
    locprec/EmissionTracker.h locprec/EmissionTracker.cpp \
    locprec/SpotMeter.cpp locprec/NoiseMeter.cpp \
    locprec/FluorophoreDistributions_Random.cpp \
    locprec/FluorophoreDistributions_Lattice.cpp \
    locprec/FluorophoreDistributions_Lines.cpp \
    locprec/FluorophoreDistribution.h \
    locprec/FluorophoreDistributions.h \
    locprec/RegionSegmenter.h locprec/NoiseSource.h\
    locprec/NoiseGenerator.h locprec/Fluorophore.h locprec/relate.h \
    locprec/FillholeSmoother.h locprec/FillholeSmoother.cpp \
    locprec/Reconstruction.hh locprec/Reconstruction.cc \
    locprec/PrecisionEstimator.cpp locprec/PrecisionEstimator.h \
    locprec/RegionOfInterest.cpp locprec/RegionOfInterest.h \
    locprec/SourceValuePrinter.cpp locprec/SourceValuePrinter.h \
    locprec/KalmanTrace.h locprec/DensityProfile.h \
    locprec/biplane_alignment/decl.h locprec/biplane_alignment/impl.h \
    locprec/biplane_alignment/Filter.cpp locprec/biplane_alignment/Source.cpp locprec/biplane_alignment/Config.cpp \
    locprec/biplane_alignment/Derivator.h locprec/biplane_alignment/Derivator.cpp \
    locprec/biplane_alignment/MotionModels.h

libdirectcamerainput_la_LDFLAGS = -no-undefined -version-info $(LIBTOOL_VERSION_INFO) -module
libdirectcamerainput_la_LIBADD = $(libb64_LIBS)
libdirectcamerainput_la_CPPFLAGS = $(libb64_CFLAGS)
libdirectcamerainput_la_SOURCES = \
    AndorCamera/CameraConnection.h AndorCamera/CameraConnection.cpp \
    AndorCamera/InputChainLink_decl.h AndorCamera/InputChainLink.h AndorCamera/InputChainLink.cpp \
    AndorCamera/LiveView.h AndorCamera/LiveView.cpp \
    AndorCamera/AndorDirect.h AndorCamera/AndorDirect.cpp AndorCamera/AndorDirect_iterator.cpp \
    AndorCamera/ViewportSelector.h AndorCamera/ViewportSelector.cpp \
    AndorCamera/plugin.cpp AndorCamera/AndorDirect_decl.h

libtestplugin_la_LDFLAGS = -no-undefined -version-info $(LIBTOOL_VERSION_INFO) -module
libtestplugin_la_SOURCES = test-plugin/plugin.cpp test-plugin/SegmentationFault.h \
    test-plugin/MuchMemoryAllocator.h test-plugin/Exception.h test-plugin/Delayer.h \
    test-plugin/BasenamePrinter.h test-plugin/BasenamePrinter.cpp \
    test-plugin/Manager.h test-plugin/Manager.cpp \
    test-plugin/md5.h test-plugin/md5.c \
    test-plugin/DummyFileInput.h test-plugin/DummyFileInput.cpp \
    test-plugin/DummyFitter.h test-plugin/DummyFitter.cpp \
    test-plugin/Verbose.h \
    test-plugin/VerboseInputFilter_decl.h test-plugin/VerboseInputFilter.h test-plugin/VerboseInputFilter.cpp \
    test-plugin/FixedPositionSpotFinder.h test-plugin/RepeatTrigger.h test-plugin/SmoothedImageSave.h

fit_camera_noise_SOURCES = locprec/fitNoise.cpp
fit_camera_noise_LDADD = liblocprec.la 
gradient_SOURCES = locprec/gradient.cpp
gradient_LDADD = liblocprec.la 
checkplugin_locprec_SOURCES = locprec/check_plugin.cpp 
checkplugin_locprec_LDADD = liblocprec.la 
determine_clusters_from_centers_SOURCES = locprec/determine-clusters-from-centers.cpp
determine_clusters_from_centers_LDADD = liblocprec.la $(librapidstormfile_LIBS)
determine_clusters_from_centers_CPPFLAGS = $(librapidstormfile_CFLAGS)
reconstruction_SOURCES = locprec/test/Reconstruction.cpp
testDisplay_SOURCES = wxDisplay/test.cpp
testDisplay_LDADD = libdStorm_display.la
check_resolutionsetter_SOURCES = inputs/ResolutionSetter_check.cpp
check_resolutionsetter_LDADD = libinputs.la
checkplugin_camera_SOURCES = AndorCamera/check_plugin.cpp
checkplugin_camera_LDADD = libdirectcamerainput.la 
checkplugin_camera_LDFLAGS = -no-undefined
checkplugin_testplugin_SOURCES = test-plugin/check_plugin.cpp
checkplugin_testplugin_LDADD = libtestplugin.la 

dstorm_@DSTORM_LIB_VERSION@_SOURCES = \
    dstorm.cpp \
    CommandLine.h CommandLine.cpp \
    ModuleLoader.h ModuleLoader.cpp \
    LibraryHandle.h LibraryHandle.cpp \
    JobStarter.h JobStarter.cpp \
    InputStream.h InputStream.cpp \
    debug.h debug_print.h $(RESOURCE_FILE) \
    TIFFOperation.h TIFFOperation.cpp \
    Car_decl.h Car.cpp Car.h 
dstorm_@DSTORM_LIB_VERSION@_LDADD = liboutputs.la libviewer.la \
    libspotFinders.la \
    libengine_stm.la \
    libengine.la libnoopengine.la \
    libinputs.la \
    libdStorm_display.la \
    -lltdl 

