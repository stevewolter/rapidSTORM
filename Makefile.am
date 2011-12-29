ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

plugin_libdir = $(DSTORM_PLUGIN_DIR)
javadir=$(datadir)/java
javabuilddir = classes
applicationsdir=$(datadir)/applications

C_TESTS = checkplugin_locprec reconstruction checkplugin_camera \
    Pixel_test \
    check_localization_variable check_expression_parser \
    check_image_slicing check_image_iterator \
    check_traits check_unittable unit_tests

bin_PROGRAMS = dstorm-@DSTORM_LIB_VERSION@ align-biplane-rapidstorm@DSTORM_LIB_VERSION@
bin_SCRIPTS = rapidstorm@DSTORM_LIB_VERSION@
check_PROGRAMS = $(C_TESTS)
lib_LTLIBRARIES = libdStorm.la librapidstormfile.la
noinst_LTLIBRARIES = libcode.la 
plugin_lib_LTLIBRARIES = liblocprec.la libdirectcamerainput.la
if CAN_BUILD_TWIDDLER
java_DATA = rapidSTORM@DSTORM_LIB_VERSION@.jar 
applications_DATA=rapidstorm@DSTORM_LIB_VERSION@.desktop
endif
doc_DATA = $(srcdir)/README 
pkgdata_DATA = 

JAVASOURCES = DStorm.java DefaultPath.java
JARCONTENT = logo.png

EXTRA_PROGRAMS = gradient determine-clusters-from-centers fit-camera-noise testDisplay 
EXTRA_DIST = DStorm.java DefaultPath.java.in $(JARCONTENT)  windows_resources.rc dStorm/windows_resources.rc
CLEANFILES = rapidSTORM@DSTORM_LIB_VERSION@.jar \
	rapidstorm@DSTORM_LIB_VERSION@.desktop rapidstorm@DSTORM_LIB_VERSION@ \
	java-built-stamp DefaultPath.java

java-built-stamp : $(JAVASOURCES)
	$(MKDIR_P) $(javabuilddir)
	$(JAVAC) -classpath $(CLASSPATH):$(TWIDDLER_JAR): -d $(javabuilddir) $^
	touch $@

DefaultPath.java : $(srcdir)/DefaultPath.java.in
	sed -e 's:[@]DSTORM_LIB_VERSION[@]:$(DSTORM_LIB_VERSION):g' \
	    -e 's:[@]DSTORM_PLUGIN_DIR[@]:$(DSTORM_PLUGIN_DIR):g' \
	    -e 's:[@]prefix[@]:$(prefix):g' \
	    -e 's:[@]sysconfdir[@]:$(sysconfdir):g' \
	    $< > $@

rapidSTORM@DSTORM_LIB_VERSION@.jar : java-built-stamp manifest $(TWIDDLER_JAR) $(JARCONTENT)
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
    RESOURCE_OBJECT = windows_resources.o
    LIBDSTORM_RESOURCE_FILE = dStorm/windows_resources.rc
    LIBDSTORM_RESOURCE_OBJECT = dStorm/windows_resources.o
    LINKER_RESOURCE = -Xlinker
else
    RESOURCE_FILE =
endif

.rc.o : 
	$(WINDRES) -I . '$<' '$@'

headerdir = $(includedir)/dStorm
nobase_include_HEADERS = \
    dStorm/InsertionPlace.h \
    dStorm/ModuleInterface.h dStorm/Localization_decl.h dStorm/Localization.h \
    dStorm/Pixel_decl.h dStorm/Pixel.h \
    dStorm/Image_decl.h dStorm/Image.h dStorm/Image_impl.h dStorm/Image_iterator.h \
    dStorm/ImageTraits_decl.h dStorm/ImageTraits.h dStorm/ImageTraits_impl.h \
    dStorm/DataSetTraits.h \
    dStorm/Job.h dStorm/JobMaster.h dStorm/UnitEntries.h dStorm/debug.h dStorm/debug_print.h dStorm/log.h dStorm/check_plugin.h \
    dStorm/Config_decl.h dStorm/Config.h \
    dStorm/Engine.h dStorm/Engine_decl.h dStorm/types/samplepos.h dStorm/types/fluorophore.h \
    dStorm/namespaces.h dStorm/pair_Eigen_traits.h \
    dStorm/output/binning/binning.h dStorm/output/binning/binning_decl.h \
    dStorm/output/binning/inversion.h \
    dStorm/output/binning/localization.h \
    dStorm/output/binning/localization_impl.h \
    dStorm/output/binning/localization_config_decl.h \
    dStorm/output/binning/localization_config.h \
    dStorm/output/binning/localization_config_impl.h \
    dStorm/output/binning/config_decl.h \
    dStorm/output/binning/config.h \
    dStorm/unit_interval.h \
    dStorm/expression/Config.h  dStorm/expression/Config_decl.h \
    dStorm/expression/Source.h  dStorm/expression/Source_decl.h \
    dStorm/stack_realign.h dStorm/dimension_names.h \
    dStorm/traits/amplitude.h dStorm/traits/base_apply.h dStorm/traits/base.h \
    dStorm/traits/covariance_matrix.h dStorm/traits/image_number.h dStorm/traits/no_range.h \
    dStorm/traits/no_resolution.h dStorm/traits/position.h dStorm/traits/range.h \
    dStorm/traits/range_impl.h dStorm/traits/residues.h dStorm/traits/resolution.h \
    dStorm/traits/scalar_check.cpp dStorm/traits/scalar.h \
    dStorm/traits/two_kernel_improvement.h dStorm/traits/tags.h dStorm/traits/scalar_iterator.h \
    dStorm/traits/optics.h dStorm/traits/image_resolution.h dStorm/traits/optics_config.h \
    dStorm/traits/fluorophore.h dStorm/traits/local_background.h \
    dStorm/traits/resolution_config.h \
    dStorm/localization/Field.h dStorm/localization/Traits_decl.h dStorm/localization/Traits.h \
    dStorm/localization/expand.h dStorm/localization/field_index_enumeration.h \
    dStorm/localization/record_decl.h dStorm/localization/record.h\
    dStorm/outputs/BinnedLocalizations.h dStorm/outputs/BinnedLocalizations_decl.h \
    dStorm/outputs/LocalizationList.h dStorm/outputs/TraceFilter.h \
    dStorm/outputs/Crankshaft.h dStorm/outputs/NullOutput.h \
    dStorm/outputs/BinnedLocalizations_converter.h \
    dStorm/outputs/BinnedLocalizations_strategies_impl.h \
    dStorm/outputs/BinnedLocalizations_strategies_config.h \
    dStorm/outputs/BinnedLocalizations_strategies.h \
    dStorm/outputs/BinnedLocalizations_impl.h \
    dStorm/output/Basename_decl.h dStorm/output/Basename.h \
    dStorm/output/BasenameAdjustedFileEntry_decl.h dStorm/output/BasenameAdjustedFileEntry.h \
    dStorm/output/Capabilities.h dStorm/output/Capabilities_decl.h \
    dStorm/output/FileOutputBuilder.h \
    dStorm/output/FilterBuilder.h dStorm/output/FilterBuilder_impl.h \
    dStorm/output/FilterSource.h \
    dStorm/output/Localizations.h dStorm/output/Localizations_iterator.h \
    dStorm/output/OutputBuilder.h dStorm/output/OutputBuilder_impl.h \
    dStorm/output/SourceFactory_decl.h dStorm/output/SourceFactory.h \
    dStorm/output/Output_decl.h dStorm/output/Output.h dStorm/output/Filter.h \
    dStorm/output/OutputSource.h \
    dStorm/output/Traits_decl.h dStorm/output/Traits.h \
    dStorm/output/TraceReducer.h \
    dStorm/output/LocalizedImage_decl.h dStorm/output/LocalizedImage.h dStorm/output/LocalizedImage_traits.h \
    dStorm/engine/Candidate_decl.h dStorm/engine/Candidate.h \
    dStorm/engine/CandidateTree_decl.h dStorm/engine/CandidateTree.h \
    dStorm/engine/Image_decl.h dStorm/engine/Image.h \
    dStorm/engine/Input_decl.h dStorm/engine/Input.h \
    dStorm/engine/Spot_decl.h dStorm/engine/Spot.h \
    dStorm/engine/SpotFinder_decl.h dStorm/engine/SpotFinder.h \
    dStorm/engine/JobInfo_decl.h dStorm/engine/JobInfo.h \
    dStorm/engine/SpotFitter_decl.h dStorm/engine/SpotFitter.h \
    dStorm/engine/SpotFitterFactory_decl.h dStorm/engine/SpotFitterFactory.h \
    dStorm/engine/SpotFitterBuilder.h \
    dStorm/input/AdapterSource.h \
    dStorm/input/Choice.h \
    dStorm/input/fwd.h dStorm/input/Link.h \
    dStorm/input/Forwarder.h \
    dStorm/input/DefaultFilterTypes.h \
    dStorm/input/MetaInfo.h \
    dStorm/input/Alternatives.h \
    dStorm/input/Source.h \
    dStorm/input/Traits.h  \
    dStorm/input/InputMutex.h \
    dStorm/input/FileInput.h \
    dStorm/helpers/thread.h dStorm/helpers/dilation.h \
    dStorm/helpers/DisplayDataSource.h dStorm/helpers/DisplayDataSource_decl.h dStorm/helpers/DisplayDataSource_impl.h \
    dStorm/helpers/DisplayManager.h \
    dStorm/helpers/dilation_impl.h dStorm/helpers/OutOfMemory.h \
    dStorm/helpers/clone_ptr.hpp dStorm/helpers/make_boost_clone.h \
    dStorm/helpers/back_inserter.h \
    dStorm/image/constructors.h dStorm/image/convert.h dStorm/image/iterator.h dStorm/image/minmax.h dStorm/image/slice.h \
    dStorm/image/mirror.h dStorm/image/crop.h dStorm/image/extend.h \
    dStorm/units/amplitude.h dStorm/units/frame_count.h \
    dStorm/units/frame_rate.h dStorm/units/megafrequency.h \
    dStorm/units/microtime.h dStorm/units/nanolength.h \
    dStorm/units/microlength.h \
    dStorm/units/nanoresolution.h dStorm/units/prefixes.h \
    dStorm/units/camera_response.h \
    dStorm/UnitEntries/FrameEntry.h dStorm/UnitEntries/Hertz.h \
    dStorm/UnitEntries/Nanometre.h dStorm/UnitEntries/PixelEntry.h \
    dStorm/UnitEntries/PixelSize.h dStorm/UnitEntries/TemperatureEntry.h \
    dStorm/UnitEntries/TimeEntry.h dStorm/UnitEntries/ADC.h \
    dStorm/localization_file/field_decl.h dStorm/localization_file/field.h \
    dStorm/localization_file/reader.h dStorm/localization_file/converter.h \
    dStorm/signals/UseSpotFinder.h dStorm/signals/UseSpotFitter.h \
    dStorm/signals/InputFileNameChange.h dStorm/signals/ResolutionChange.h dStorm/signals/BasenameChange.h

libdStorm_la_SOURCES = \
    dStorm/Localization.cpp $(LIBDSTORM_RESOURCE_FILE) \
    dStorm/JobMaster.cpp dStorm/UnitEntries.cpp \
    dStorm/DataSetTraits.cpp dStorm/units/units.cpp dStorm/ImageTraits.cpp \
    dStorm/localization/Field.cpp \
    dStorm/localization/Traits.cpp \
    dStorm/traits/traits.cpp dStorm/image/instantiate.cpp \
    dStorm/traits/optics.cpp \
    dStorm/localization/record.cpp \
    dStorm/expression/Evaluator.h dStorm/expression/Simplifier.h dStorm/expression/UnitChecker.h \
    dStorm/expression/Expression.cpp dStorm/expression/Expression.h \
    dStorm/expression/localization_variable.cpp dStorm/expression/localization_variable_decl.h \
    dStorm/expression/localization_variable.h dStorm/expression/localization_variable_impl.h \
    dStorm/expression/Parser.cpp dStorm/expression/Parser.h dStorm/expression/Parser_impl.h \
    dStorm/expression/SIPrefixes.h \
    dStorm/expression/tokens.cpp dStorm/expression/tokens_decl.h dStorm/expression/tokens.h \
    dStorm/expression/types.cpp dStorm/expression/types.h dStorm/expression/types_decl.h \
    dStorm/expression/UnitTable.cpp dStorm/expression/UnitTable.h dStorm/expression/UnitTable_impl.h \
    dStorm/expression/Config.cpp dStorm/expression/Source.cpp \
    dStorm/expression/CommandLine.h dStorm/expression/Source_filters.h \
    dStorm/expression/SimpleFilters.h dStorm/expression/SimpleFilters.cpp \
    dStorm/helpers/thread.cpp dStorm/helpers/DisplayManager.cpp dStorm/helpers/dilation.cpp \
    dStorm/helpers/OutOfMemory.cpp \
    dStorm/input/Source.cpp \
    dStorm/input/MetaInfo.cpp \
    dStorm/input/InputMutex.cpp \
    dStorm/input/Link.cpp dStorm/input/Choice.cpp \
    dStorm/input/Alternatives.cpp \
    dStorm/input/Forwarder.cpp \
    dStorm/input/Method.hpp \
    dStorm/engine/CandidateTree.cpp dStorm/engine/SpotFinder.cpp dStorm/engine/SpotFitterFactory.cpp dStorm/engine/Image.cpp \
    dStorm/engine/JobInfo.cpp \
    dStorm/output/debug.h \
    dStorm/output/Output.cpp dStorm/output/LocalizedImage.cpp \
    dStorm/output/Basename.cpp \
    dStorm/output/BasenameAdjustedFileEntry.cpp \
    dStorm/output/Config.cpp \
    dStorm/output/FilterSource.cpp \
    dStorm/output/OutputSource.cpp dStorm/output/Trace.cpp \
    dStorm/output/Traits.cpp \
    dStorm/output/Localizations.cpp \
    dStorm/output/TraceReducers.cpp \
    dStorm/output/SourceFactory.cpp  \
    dStorm/output/LocalizedImage_traits.cpp \
    dStorm/output/binning/localization.cpp \
    dStorm/output/binning/localization_config.cpp \
    dStorm/output/binning/config.cpp dStorm/output/binning/binning.cpp \
    dStorm/output/Filter.cpp \
    dStorm/outputs/BinnedLocalizations.cpp dStorm/outputs/LocalizationList.cpp dStorm/outputs/TraceFilter.cpp dStorm/outputs/Crankshaft.cpp \
    dStorm/outputs/NullOutput.cpp dStorm/outputs/BinnedLocalizations_strategies.cpp \
    dStorm/traits/resolution_config.cpp
libdStorm_la_LDFLAGS = -no-undefined -version-info @LIBTOOL_VERSION_INFO@ $(LINKER_RESOURCE) $(LIBDSTORM_RESOURCE_OBJECT)
libdStorm_la_LIBADD = $(boost_thread_LIB) 
libdStorm_la_DEPENDENCIES = $(LIBDSTORM_RESOURCE_OBJECT) 

librapidstormfile_la_SOURCES = \
    dStorm/localization_file/fields.cpp dStorm/localization_file/reader.cpp \
    dStorm/localization_file/children_field.h dStorm/localization_file/children_field.cpp \
    dStorm/localization_file/localization_field_impl.h dStorm/localization_file/localization_field.h \
    dStorm/localization_file/unknown_field.h 
librapidstormfile_la_LDFLAGS = -no-undefined -version-info @LIBTOOL_VERSION_INFO@ 
librapidstormfile_la_LIBADD = libdStorm.la $(tinyxml_LIBS)

dstorm_@DSTORM_LIB_VERSION@_SOURCES = dstorm.cpp
dstorm_@DSTORM_LIB_VERSION@_LDADD = libcode.la
dstorm_@DSTORM_LIB_VERSION@_LDFLAGS = $(LINKER_RESOURCE) $(RESOURCE_OBJECT)
dstorm_@DSTORM_LIB_VERSION@_CPPFLAGS = $(GraphicsMagick_CFLAGS) $(WX_CPPFLAGS) "-DRAPIDSTORM_PLUGIN_DIR=\"$(DSTORM_PLUGIN_DIR)\""

libcode_la_CXXFLAGS = $(WX_CXXFLAGS)
libcode_la_CPPFLAGS = $(GraphicsMagick_CFLAGS) $(WX_CPPFLAGS) "-DRAPIDSTORM_PLUGIN_DIR=\"$(DSTORM_PLUGIN_DIR)\""
libcode_la_LIBADD = libdStorm.la $(GraphicsMagick_LIBS) $(WX_LIBS)
libcode_la_LIBADD += $(boost_system_LIB) $(boost_thread_LIB) 
libcode_la_LIBADD += librapidstormfile.la $(tinyxml_LIBS)
libcode_la_SOURCES = \
    dstorm.cpp \
    CommandLine.h CommandLine.cpp \
    ModuleLoader.h ModuleLoader.cpp \
    LibraryHandle.h LibraryHandle.cpp \
    JobStarter.h JobStarter.cpp \
    InputStream.h InputStream.cpp \
    debug.h debug_print.h $(RESOURCE_FILE) \
    TIFFOperation.h TIFFOperation.cpp

libcode_la_SOURCES += \
    job/Car.cpp job/Car.h job/ComputationThread.h \
    job/Config.h job/Config.cpp \
    job/Queue.h job/Queue.cpp \
    job/Run.h job/Run.cpp

libcode_la_SOURCES += \
    dStorm/output/Config.h

libcode_la_SOURCES += \
    inputs/inputs.h inputs/inputs.cpp \
    inputs/FileMethod.h inputs/FileMethod.cpp \
    inputs/InputMethods.h inputs/InputMethods.cpp \
    inputs/EngineChoice.cpp inputs/EngineChoice.h \
    inputs/InputBase.cpp inputs/InputBase.h \
    inputs/InsertionPoint.cpp inputs/InsertionPoint.h \
    inputs/join.h inputs/join.cpp \
    inputs/join/iterator_decl.hpp inputs/join/iterator_generic.hpp \
    inputs/join/iterator.hpp inputs/join/iterator.cpp \
    inputs/join/spatial.hpp inputs/join/spatial.cpp \
    inputs/join/temporal.hpp inputs/join/temporal.cpp \
    inputs/TIFF.h inputs/TIFF.cpp inputs/TIFF_OpenFile.cpp \
    inputs/BackgroundDeviationEstimator.cpp \
    inputs/BackgroundDeviationEstimator.h \
    inputs/Splitter.h inputs/Splitter.cpp \
    inputs/YMirror.h inputs/YMirror.cpp \
    inputs/ResolutionSetter.h \
    inputs/ResolutionSetter.h inputs/ResolutionSetter.cpp \
    inputs/SampleInfo.h inputs/SampleInfo.cpp \
    inputs/ROIFilter.h inputs/ROIFilter.cpp \
    inputs/Basename.h inputs/Basename.cpp \
    inputs/Buffer.h inputs/Buffer.cpp \
    inputs/LocalizationFile.h inputs/LocalizationFile.cpp 

libcode_la_SOURCES += \
	noop_engine/ChainLink.cpp noop_engine/ChainLink_decl.h \
	noop_engine/ChainLink.h noop_engine/Engine.cpp noop_engine/Engine.h

libcode_la_SOURCES += \
    engine/Config.cpp engine/Config_decl.h engine/Config.h \
    engine/Engine.cpp \
    engine/EngineDebug.h engine/Engine.h \
    engine/ImageFitter.h \
    engine/SpotFinder.cpp engine/debug.h  \
    engine/ChainLink_decl.h engine/ChainLink.h engine/ChainLink.cpp \
    engine/PlaneFlattener.h engine/PlaneFlattener.cpp

libcode_la_SOURCES += \
	    engine_stm/LocalizationBuncher.cpp engine_stm/LocalizationBuncher.h \
	    engine_stm/Config.h engine_stm/Config.cpp \
	    engine_stm/ChainLink_decl.h engine_stm/ChainLink.h engine_stm/ChainLink.cpp

libcode_la_SOURCES += \
    outputs/AverageImage.cpp outputs/AverageImage.h outputs/BasicTransmissions.cpp \
    outputs/LocalizationCounter.cpp outputs/LocalizationCounter.h \
    outputs/ProgressMeter.cpp \
    outputs/ProgressMeter.h outputs/RawImageFile.cpp outputs/RawImageFile.h outputs/Slicer.cpp \
    outputs/Slicer.h \
    outputs/BasicTransmissions.h \
    outputs/MemoryCache.cpp outputs/MemoryCache.h \
    outputs/MemoryCache_Cache.h outputs/MemoryCache_Cache.cpp \
    outputs/LocalizationFile.h outputs/LocalizationFile.cpp

libcode_la_SOURCES += \
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

libcode_la_SOURCES += \
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

libcode_la_SOURCES += test-plugin/plugin.cpp test-plugin/plugin.h test-plugin/SegmentationFault.h \
    test-plugin/MuchMemoryAllocator.h test-plugin/Exception.h test-plugin/Delayer.h \
    test-plugin/BasenamePrinter.h test-plugin/BasenamePrinter.cpp \
    test-plugin/Manager.h test-plugin/Manager.cpp \
    test-plugin/md5.h test-plugin/md5.c \
    test-plugin/DummyFileInput.h test-plugin/DummyFileInput.cpp \
    test-plugin/DummyFitter.h test-plugin/DummyFitter.cpp \
    test-plugin/Verbose.h \
    test-plugin/VerboseInputFilter_decl.h test-plugin/VerboseInputFilter.h test-plugin/VerboseInputFilter.cpp \
    test-plugin/FixedPositionSpotFinder.h test-plugin/RepeatTrigger.h test-plugin/SmoothedImageSave.h

libcode_la_SOURCES += \
    wxDisplay/wxManager.h wxDisplay/wxManager.cpp wxDisplay/App.h wxDisplay/App.cpp \
    wxDisplay/Window.h wxDisplay/Window.cpp wxDisplay/Canvas.h wxDisplay/Canvas.cpp \
    wxDisplay/ZoomSlider.h wxDisplay/ZoomSlider.cpp wxDisplay/Key.h wxDisplay/Key.cpp \
    wxDisplay/helpers.h wxDisplay/ScaleBar.h wxDisplay/ScaleBar.cpp wxDisplay/helpers.cpp \
    wxDisplay/wxManager_store_image.cpp wxDisplay/SizeConvert.h

liblocprec_la_LDFLAGS = -no-undefined -version-info $(LIBTOOL_VERSION_INFO) -module
liblocprec_la_LIBADD = libdStorm.la $(gsl_LIBS) $(boost_thread_LIB)
liblocprec_la_DEPENDENCIES = libdStorm.la
liblocprec_la_CPPFLAGS = $(gsl_CFLAGS)
liblocprec_la_SOURCES = \
    locprec/plugin.cpp locprec/foreach.h locprec/NoiseSource.cpp \
    locprec/NoiseMeter.h locprec/Fluorophore.cpp \
    locprec/pixelatedBessel.h locprec/pixelatedBessel.cpp \
    locprec/NoiseGenerator.cpp locprec/Counter.h locprec/Noise.h locprec/SpotMeter.h \
    locprec/RegionSegmenter.cpp \
    locprec/EmissionTracker_decl.h locprec/EmissionTracker.h locprec/EmissionTracker.cpp \
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
    locprec/DistanceHistogram.h locprec/DistanceHistogram.cpp \
    locprec/RipleyK.h locprec/RipleyK.cpp \
    locprec/VarianceEstimator.h locprec/VarianceEstimator.cpp

libdirectcamerainput_la_LDFLAGS = -no-undefined -version-info $(LIBTOOL_VERSION_INFO) -module
libdirectcamerainput_la_LIBADD = libdStorm.la $(libb64_LIBS) $(boost_system_LIB) $(boost_thread_LIB)
libdirectcamerainput_la_CPPFLAGS = $(libb64_CFLAGS)
libdirectcamerainput_la_SOURCES = \
    AndorCamera/CameraConnection.h AndorCamera/CameraConnection.cpp \
    AndorCamera/InputChainLink_decl.h AndorCamera/InputChainLink.h AndorCamera/InputChainLink.cpp \
    AndorCamera/LiveView.h AndorCamera/LiveView.cpp \
    AndorCamera/AndorDirect.h AndorCamera/AndorDirect.cpp AndorCamera/AndorDirect_iterator.cpp \
    AndorCamera/ViewportSelector.h AndorCamera/ViewportSelector.cpp \
    AndorCamera/plugin.cpp AndorCamera/AndorDirect_decl.h

fit_camera_noise_SOURCES = locprec/fitNoise.cpp
fit_camera_noise_LDADD = liblocprec.la 
gradient_SOURCES = locprec/gradient.cpp
gradient_LDADD = liblocprec.la 
checkplugin_locprec_SOURCES = locprec/check_plugin.cpp 
checkplugin_locprec_LDADD = liblocprec.la 
determine_clusters_from_centers_SOURCES = locprec/determine-clusters-from-centers.cpp
determine_clusters_from_centers_LDADD = liblocprec.la $(tinyxml_LIBS)
determine_clusters_from_centers_CPPFLAGS = $(tinyxml_CFLAGS)
reconstruction_SOURCES = locprec/test/Reconstruction.cpp
testDisplay_SOURCES = wxDisplay/test.cpp
testDisplay_LDADD = libdStorm_display.la
checkplugin_camera_SOURCES = AndorCamera/check_plugin.cpp
checkplugin_camera_LDADD = libdirectcamerainput.la 
checkplugin_camera_LDFLAGS = -no-undefined
Pixel_test_SOURCES = dStorm/Pixel_test.cpp
check_traits_SOURCES = dStorm/traits/scalar_check.cpp
check_traits_LDADD = libdStorm.la
check_unittable_SOURCES = dStorm/expression/UnitTable_check.cpp 
check_unittable_LDADD = libdStorm.la
check_localization_variable_SOURCES = dStorm/expression/localization_variable_check.cpp
check_localization_variable_LDADD = libdStorm.la
check_expression_parser_SOURCES = dStorm/expression/Parser_test.cpp
check_expression_parser_LDADD = libdStorm.la
check_image_slicing_SOURCES = dStorm/image/slice_check.cpp
check_image_slicing_LDADD = libdStorm.la
check_image_iterator_SOURCES = dStorm/image/iterator_check.cpp
check_image_iterator_LDADD = libdStorm.la

unit_tests_SOURCES = unit_tests.cpp inputs/TIFF.cpp inputs/TIFF_OpenFile.cpp TIFFOperation.cpp dejagnu.h
unit_tests_LDADD = libcode.la 

align_biplane_rapidstorm@DSTORM_LIB_VERSION@_SOURCES = alignment_fitter.cpp
align_biplane_rapidstorm@DSTORM_LIB_VERSION@_CPPFLAGS = $(gsl_CFLAGS)
align_biplane_rapidstorm@DSTORM_LIB_VERSION@_LDADD = librapidstormfile.la libdStorm.la $(gsl_LIBS) 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = librapidstorm.pc librapidstormfile.pc
