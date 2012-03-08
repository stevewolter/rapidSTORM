ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects

# Doxygen support
include aminclude.am
MOSTLYCLEANFILES = $(DX_CLEANFILES)

plugin_libdir = $(DSTORM_PLUGIN_DIR)
javadir=$(datadir)/java
javabuilddir = classes
applicationsdir=$(datadir)/applications

C_TESTS = dstorm-@DSTORM_LIB_VERSION@

bin_PROGRAMS = dstorm-@DSTORM_LIB_VERSION@ align-biplane-rapidstorm@DSTORM_LIB_VERSION@
bin_SCRIPTS = rapidstorm@DSTORM_LIB_VERSION@
check_PROGRAMS = $(C_TESTS)
lib_LTLIBRARIES = libdStorm.la librapidstormfile.la
if CAN_BUILD_TWIDDLER
java_DATA = rapidSTORM@DSTORM_LIB_VERSION@.jar 
applications_DATA=rapidstorm@DSTORM_LIB_VERSION@.desktop
endif
doc_DATA = $(srcdir)/README 
pkgdata_DATA = 

JAVASOURCES = DStorm.java DefaultPath.java
JARCONTENT = logo.png

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
    dStorm/polynomial_3d.h \
    dStorm/InsertionPlace.h \
    dStorm/ModuleInterface.h dStorm/Localization_decl.h dStorm/Localization.h \
    dStorm/Pixel_decl.h dStorm/Pixel.h \
    dStorm/DataSetTraits.h \
    dStorm/Job.h dStorm/JobMaster.h dStorm/UnitEntries.h dStorm/debug.h dStorm/debug_print.h dStorm/log.h dStorm/check_plugin.h \
    dStorm/Config_decl.h dStorm/Config.h \
    dStorm/Engine.h dStorm/Engine_decl.h dStorm/types/samplepos.h dStorm/types/fluorophore.h \
    dStorm/namespaces.h dStorm/pair_Eigen_traits.h \
    dStorm/output/binning/binning.h dStorm/output/binning/binning.hpp dStorm/output/binning/binning_decl.h \
    dStorm/output/binning/inversion.h \
    dStorm/output/binning/localization.h \
    dStorm/output/binning/localization_impl.h \
    dStorm/output/binning/localization_config_decl.h \
    dStorm/output/binning/localization_config.h \
    dStorm/output/binning/localization_config_impl.h \
    dStorm/output/binning/config_decl.h \
    dStorm/output/binning/config.h \
    dStorm/output/binning/dummy_binner.h \
    dStorm/output/binning/constant_binner.h \
    dStorm/unit_interval.h \
    dStorm/stack_realign.h dStorm/dimension_names.h \
    dStorm/traits/amplitude.h dStorm/traits/base_apply.h dStorm/traits/base.h \
    dStorm/traits/covariance_matrix.h dStorm/traits/image_number.h dStorm/traits/no_range.h \
    dStorm/traits/no_resolution.h dStorm/traits/position.h dStorm/traits/range.h \
    dStorm/traits/range_impl.h dStorm/traits/residues.h dStorm/traits/resolution.h \
    dStorm/traits/scalar_check.cpp dStorm/traits/scalar.h \
    dStorm/traits/two_kernel_improvement.h dStorm/traits/tags.h dStorm/traits/scalar_iterator.h \
    dStorm/traits/Projection.h dStorm/traits/ProjectionConfig.h dStorm/traits/ScaledProjection.h \
    dStorm/traits/optics.h dStorm/traits/image_resolution.h dStorm/traits/optics_config.h \
    dStorm/traits/fluorophore.h dStorm/traits/local_background.h \
    dStorm/traits/resolution_config.h dStorm/traits/DepthInfo.h \
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
    dStorm/engine/FitPosition.h \
    dStorm/engine/SpotFitterFactory_decl.h dStorm/engine/SpotFitterFactory.h \
    dStorm/engine/SpotFitterBuilder.h \
    dStorm/engine/InputTraits.h dStorm/engine/InputPlane.h \
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
    dStorm/helpers/thread.h \
    dStorm/helpers/OutOfMemory.h \
    dStorm/helpers/clone_ptr.hpp dStorm/helpers/make_boost_clone.h \
    dStorm/helpers/back_inserter.h \
    dStorm/Image.h dStorm/Image_impl.h dStorm/Image_iterator.h \
    dStorm/image/Image.h dStorm/image/Image.hpp \
    dStorm/image/fwd.h dStorm/image/MetaInfo.h dStorm/image/MetaInfo.hpp \
    dStorm/image/constructors.h dStorm/image/convert.h dStorm/image/iterator.h dStorm/image/minmax.h dStorm/image/slice.h \
    dStorm/image/mirror.h dStorm/image/crop.h dStorm/image/extend.h \
    dStorm/image/dilation.h dStorm/image/dilation_impl.h dStorm/image/contains.h \
    dStorm/image/find_by_offset.hpp dStorm/image/normalize.h \
    dStorm/image/corners.h \
    dStorm/image/morphological_reconstruction.h \
    dStorm/image/morphological_reconstruction.hpp \
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
    dStorm/signals/InputFileNameChange.h dStorm/signals/ResolutionChange.h dStorm/signals/BasenameChange.h \
    dStorm/display/fwd.h \
    dStorm/display/DataSource.h dStorm/display/display_normalized.hpp \
    dStorm/display/Manager.h \
    dStorm/display/DummyManager.h  \
    dStorm/Direction.h

libdStorm_la_SOURCES = \
    dStorm/Localization.cpp $(LIBDSTORM_RESOURCE_FILE) \
    dStorm/JobMaster.cpp dStorm/UnitEntries.cpp \
    dStorm/DataSetTraits.cpp dStorm/units/units.cpp dStorm/image/MetaInfo.cpp \
    dStorm/localization/Field.cpp \
    dStorm/localization/Traits.cpp \
    dStorm/traits/traits.cpp dStorm/image/instantiate.cpp \
    dStorm/traits/optics.cpp \
    dStorm/traits/unit_tests.h dStorm/traits/unit_tests.cpp \
    dStorm/traits/Projection.cpp \
    dStorm/traits/AffineProjection.cpp dStorm/traits/ScaledProjection.cpp \
    dStorm/traits/SupportPointProjection.cpp \
    dStorm/traits/DepthInfo.cpp \
    dStorm/localization/record.cpp \
    dStorm/helpers/thread.cpp dStorm/display/Manager.cpp dStorm/image/dilation.cpp \
    dStorm/helpers/OutOfMemory.cpp \
    dStorm/input/Source.cpp \
    dStorm/input/MetaInfo.cpp \
    dStorm/input/InputMutex.cpp \
    dStorm/input/Link.cpp dStorm/input/Choice.cpp \
    dStorm/input/Alternatives.cpp \
    dStorm/input/Forwarder.cpp \
    dStorm/input/Method.hpp \
    dStorm/engine/CandidateTree.cpp dStorm/engine/SpotFinder.cpp dStorm/engine/SpotFitterFactory.cpp dStorm/engine/Image.cpp \
    dStorm/engine/JobInfo.cpp dStorm/engine/InputTraits.cpp \
    dStorm/engine/InputPlane.cpp \
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
    dStorm/output/binning/constant_binner.cpp \
    dStorm/output/Filter.cpp \
    dStorm/outputs/BinnedLocalizations.cpp dStorm/outputs/LocalizationList.cpp dStorm/outputs/TraceFilter.cpp dStorm/outputs/Crankshaft.cpp \
    dStorm/outputs/NullOutput.cpp dStorm/outputs/BinnedLocalizations_strategies.cpp \
    dStorm/traits/resolution_config.cpp \
    dStorm/display/DummyManager.cpp \
    dStorm/Pixel_test.cpp \
    dStorm/image/slice_check.cpp dStorm/image/iterator_check.cpp \
    dStorm/traits/scalar_check.cpp \
    dStorm/image/morphological_reconstruction.cpp
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
dstorm_@DSTORM_LIB_VERSION@_LDFLAGS = $(LINKER_RESOURCE) $(RESOURCE_OBJECT) -fwhole-program 
dstorm_@DSTORM_LIB_VERSION@_DEPENDENCIES = $(RESOURCE_OBJECT) libdStorm.la librapidstormfile.la
dstorm_@DSTORM_LIB_VERSION@_CPPFLAGS = $(GraphicsMagick_CFLAGS) $(WX_CPPFLAGS) "-DRAPIDSTORM_PLUGIN_DIR=\"$(DSTORM_PLUGIN_DIR)\""
dstorm_@DSTORM_LIB_VERSION@_CXXFLAGS = $(WX_CXXFLAGS) $(gsl_CFLAGS) $(libb64_CFLAGS)
dstorm_@DSTORM_LIB_VERSION@_LDADD = libdStorm.la $(GraphicsMagick_LIBS) $(WX_LIBS) 
dstorm_@DSTORM_LIB_VERSION@_LDADD += $(boost_system_LIB) $(boost_thread_LIB) $(libb64_LIBS) 
dstorm_@DSTORM_LIB_VERSION@_LDADD += librapidstormfile.la $(tinyxml_LIBS) $(gsl_LIBS)

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    CommandLine.h CommandLine.cpp \
    ModuleLoader.h ModuleLoader.cpp \
    LibraryHandle.h LibraryHandle.cpp \
    JobStarter.h JobStarter.cpp \
    InputStream.h InputStream.cpp \
    debug.h debug_print.h $(RESOURCE_FILE)

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    job/Car.cpp job/Car.h \
    job/Config.h job/Config.cpp \
    job/Queue.h job/Queue.cpp \
    job/Run.h job/Run.cpp \
    job/Control.h job/Control.cpp

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    dStorm/output/Config.h

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
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

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
	noop_engine/ChainLink.cpp noop_engine/ChainLink_decl.h \
	noop_engine/ChainLink.h noop_engine/Engine.cpp noop_engine/Engine.h

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    engine/Config.cpp engine/Config_decl.h engine/Config.h \
    engine/Engine.cpp \
    engine/EngineDebug.h engine/Engine.h \
    engine/ImageFitter.h \
    engine/SpotFinder.cpp engine/debug.h  \
    engine/ChainLink_decl.h engine/ChainLink.h engine/ChainLink.cpp \
    engine/PlaneFlattener.h engine/PlaneFlattener.cpp

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
	    engine_stm/LocalizationBuncher.cpp engine_stm/LocalizationBuncher.h \
	    engine_stm/ChainLink.h engine_stm/ChainLink.cpp

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    expression/Config.h  expression/Config_decl.h \
    expression/Source.h  expression/Source_decl.h \
    expression/Evaluator.h expression/Simplifier.h expression/UnitChecker.h \
    expression/Expression.cpp expression/Expression.h \
    expression/localization_variable.cpp expression/localization_variable_decl.h \
    expression/localization_variable.h expression/localization_variable_impl.h \
    expression/Parser.cpp expression/Parser.h expression/Parser_impl.h \
    expression/SIPrefixes.h \
    expression/tokens.cpp expression/tokens_decl.h expression/tokens.h \
    expression/types.cpp expression/types.h expression/types_decl.h \
    expression/UnitTable.cpp expression/UnitTable.h expression/UnitTable_impl.h \
    expression/Config.cpp expression/Source.cpp \
    expression/CommandLine.h expression/Source_filters.h \
    expression/SimpleFilters.h expression/SimpleFilters.cpp \
    expression/unit_test.cpp

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    outputs/AverageImage.cpp outputs/AverageImage.h outputs/BasicTransmissions.cpp \
    outputs/LocalizationCounter.cpp outputs/LocalizationCounter.h \
    outputs/ProgressMeter.cpp \
    outputs/ProgressMeter.h \
    outputs/Slicer.cpp outputs/Slicer.h \
    outputs/BasicTransmissions.h \
    outputs/MemoryCache.cpp outputs/MemoryCache.h \
    outputs/MemoryCache_Cache.h outputs/MemoryCache_Cache.cpp \
    outputs/LocalizationFile.h outputs/LocalizationFile.cpp

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    spotFinders/averageSmooth.cpp \
    spotFinders/averageSmooth.h \
    spotFinders/ErosionSmoother.h \
    spotFinders/ErosionSmoother.cpp \
    spotFinders/GaussSmoothing.cpp \
    spotFinders/GaussSmoothing.h \
    spotFinders/Median.cpp \
    spotFinders/MedianSmoother.h \
    spotFinders/MedianSmoother.cpp \
    spotFinders/Spalttiefpass.h spotFinders/Spalttiefpass.cpp \
    spotFinders/Fillhole.h spotFinders/Fillhole.cpp \
    spotFinders/spotFinders.cpp spotFinders/spotFinders.h

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    viewer/fwd.h \
    viewer/Image.h viewer/Image.cpp \
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
    viewer/TerminalCache.h viewer/TerminalCache_inline.h viewer/TerminalCache.cpp \
    viewer/BinnedLocalizations.cpp \
    viewer/Status_decl.h viewer/Status.h viewer/Status.cpp \
    viewer/LiveBackend_converter.h viewer/LiveBackend_converter.cpp \
    viewer/TerminalBackend_converter.h viewer/TerminalBackend_converter.cpp \
    viewer/ImageDiscretizer_converter.h \
    viewer/plugin.h viewer/plugin.cpp \
    viewer/ColourScheme_decl.h viewer/ColourScheme.h  \
    viewer/colour_schemes/base.cpp viewer/colour_schemes/base_impl.h viewer/colour_schemes/base.h \
    viewer/colour_schemes/decl.h viewer/colour_schemes/impl.h viewer/colour_schemes/instantiate.h \
    viewer/colour_schemes/hot_config.cpp viewer/colour_schemes/hot_config.h viewer/colour_schemes/hot.h \
    viewer/colour_schemes/mono_config.cpp viewer/colour_schemes/mono_config.h viewer/colour_schemes/mono.h \
    viewer/colour_schemes/colored_config.cpp viewer/colour_schemes/colored_config.h viewer/colour_schemes/colored.h \
    viewer/colour_schemes/coordinate_config.cpp viewer/colour_schemes/coordinate_config.h viewer/colour_schemes/coordinate.h viewer/colour_schemes/coordinate.cpp \
    viewer/colour_schemes/HueSaturationMixer.h viewer/colour_schemes/HueSaturationMixer.cpp \
    viewer/UnitTest.cpp

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    test-plugin/plugin.cpp test-plugin/plugin.h test-plugin/SegmentationFault.h \
    test-plugin/MuchMemoryAllocator.h test-plugin/Exception.h test-plugin/Delayer.h \
    test-plugin/BasenamePrinter.h test-plugin/BasenamePrinter.cpp \
    test-plugin/Manager.h test-plugin/Manager.cpp \
    test-plugin/md5.h test-plugin/md5.c \
    test-plugin/DummyFileInput.h test-plugin/DummyFileInput.cpp \
    test-plugin/DummyFitter.h test-plugin/DummyFitter.cpp \
    test-plugin/Verbose.h \
    test-plugin/VerboseInputFilter_decl.h test-plugin/VerboseInputFilter.h test-plugin/VerboseInputFilter.cpp \
    test-plugin/FixedPositionSpotFinder.h test-plugin/RepeatTrigger.h test-plugin/SmoothedImageSave.h \
    test-plugin/xenophon.h test-plugin/xenophon.cpp \
    test-plugin/cpu_time.h test-plugin/cpu_time.cpp

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    wxDisplay/fwd.h \
    wxDisplay/wxManager.h wxDisplay/wxManager.cpp wxDisplay/App.h wxDisplay/App.cpp \
    wxDisplay/Window.h wxDisplay/Window.cpp wxDisplay/Canvas.h wxDisplay/Canvas.cpp \
    wxDisplay/ZoomSlider.h wxDisplay/ZoomSlider.cpp wxDisplay/Key.h wxDisplay/Key.cpp \
    wxDisplay/helpers.h wxDisplay/ScaleBar.h wxDisplay/ScaleBar.cpp wxDisplay/helpers.cpp \
    wxDisplay/wxManager_store_image.cpp wxDisplay/SizeConvert.h

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    guf/form_fitter/Output.h guf/form_fitter/Output.cpp \
    guf/form_fitter/Fitter.h guf/form_fitter/Fitter.cpp \
    guf/form_fitter/Config.h guf/form_fitter/Config.cpp \
    guf/form_fitter/decl.h guf/form_fitter/Tile.h \
    guf/form_fitter/GUI.h guf/form_fitter/GUI.cpp \
    guf/form_fitter/Input.h guf/form_fitter/Input.cpp \
    guf/form_fitter/LocalizationValueFinder.h guf/form_fitter/LocalizationValueFinder.cpp

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    nonlinfit/AbstractFunction.h \
    nonlinfit/AbstractMoveable.h \
    nonlinfit/AbstractTerminator.h \
    nonlinfit/AbstractFunctionAdapter.h \
    nonlinfit/access_parameters.hpp \
    nonlinfit/append.h \
    nonlinfit/BoundFunction.h nonlinfit/BoundFunction.hpp \
    nonlinfit/DerivationSummand.h \
    nonlinfit/Evaluation.h \
    nonlinfit/Evaluator.h \
    nonlinfit/derive_by.hpp \
    nonlinfit/Bind.h nonlinfit/Bind.hpp \
    nonlinfit/FunctionConverter.h \
    nonlinfit/VectorPosition.h nonlinfit/VectorPosition.hpp \
    nonlinfit/fwd.h \
    nonlinfit/index_of.h \
    nonlinfit/make_bitset.h \
    nonlinfit/make_functor.hpp \
    nonlinfit/Lambda.h \
    nonlinfit/Terminator.h \
    nonlinfit/TermParameter.h \
    nonlinfit/Evaluation.hpp \
    nonlinfit/Xs.h \
    nonlinfit/Xs.hpp \
    nonlinfit/functions/Constant.h \
    nonlinfit/functions/Polynom.h \
    nonlinfit/functions/Zero.h \
    nonlinfit/levmar/Config.h \
    nonlinfit/levmar/Fitter.hpp \
    nonlinfit/levmar/Fitter.h \
    nonlinfit/levmar/exceptions.h \
    nonlinfit/levmar/fwd.h \
    nonlinfit/levmar/Fitter.cpp \
    nonlinfit/plane/fwd.h \
    nonlinfit/plane/check_evaluator.hpp \
    nonlinfit/plane/ComputationWay.h \
    nonlinfit/plane/Distance.h nonlinfit/plane/Distance.hpp \
    nonlinfit/plane/Joint.h \
    nonlinfit/plane/Disjoint.h nonlinfit/plane/Disjoint.hpp \
    nonlinfit/plane/JointData.h \
    nonlinfit/plane/DisjointData.h nonlinfit/plane/DisjointData.hpp \
    nonlinfit/plane/GenericData.h \
    nonlinfit/plane/JointData.hpp \
    nonlinfit/plane/Jacobian.h \
    nonlinfit/terminators/RelativeChange.h nonlinfit/terminators/RelativeChange.cpp \
    nonlinfit/terminators/StepLimit.h nonlinfit/terminators/StepLimit.cpp \
    nonlinfit/terminators/All.h \
    nonlinfit/terminators/fwd.h \
    nonlinfit/sum/AbstractFunction.h \
    nonlinfit/sum/AbstractFunction.hpp \
    nonlinfit/sum/AbstractMap.h \
    nonlinfit/sum/AbstractMap.hpp \
    nonlinfit/sum/Evaluator.h \
    nonlinfit/sum/Evaluator.hpp \
    nonlinfit/sum/fwd.h \
    nonlinfit/sum/Lambda.h

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    guf/fitter.h guf/fitter.cpp \
    guf/guf/EvaluationTags.h \
    guf/guf/mle_converter.h \
    guf/guf/Config.cpp guf/guf/Config_decl.h guf/guf/Config.h \
    guf/guf/DataCube.cpp guf/guf/DataCube.h \
    guf/guf/DataPlane.cpp guf/guf/DataPlane.h \
    guf/guf/FunctionRepository.h guf/guf/FunctionRepository.hpp \
    guf/guf/Factory.cpp guf/guf/Factory.h \
    guf/guf/FitAnalysis.h \
    guf/guf/FitPosition.hpp \
    guf/guf/fit_position_out_of_range.h \
    guf/guf/Fitter.cpp guf/guf/Fitter.h \
    guf/guf/InitialValueFinder.h guf/guf/InitialValueFinder.cpp \
    guf/guf/TraitValueFinder.h guf/guf/TraitValueFinder.cpp \
    guf/guf/NaiveFitter.h guf/guf/NaiveFitter.cpp \
    guf/guf/ModelledFitter.h guf/guf/ModelledFitter.hpp \
    guf/guf/LocalizationChecker.h guf/guf/LocalizationChecker.cpp \
    guf/guf/LocalizationCreator.h guf/guf/LocalizationCreator.cpp \
    guf/guf/Data_fwd.h \
    guf/guf/KernelCreator.h guf/guf/KernelCreator.cpp \
    guf/guf/FitTerminator.h \
    guf/guf/TransformedImage.hpp \
    guf/guf/Statistics.h \
    guf/guf/TransformedImage_cut_region.h \
    guf/guf/PlaneFunction.h guf/guf/PlaneFunction.hpp \
    guf/guf/DistanceMetric.h guf/guf/InputPlane.h \
    guf/guf/Spot.h \
    guf/guf/Centroid.h guf/guf/Centroid.cpp \
    guf/guf/TransformedImage.h \
    guf/unit_tests.h

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    guf/psf/fwd.h guf/psf/parameters.h guf/psf/parameters.hpp \
    guf/psf/BaseEvaluator.h guf/psf/BaseEvaluator.cpp \
    guf/psf/DisjointEvaluator.h \
    guf/psf/Derivable.h \
    guf/psf/free_form.h \
    guf/psf/fixed_form.h \
    guf/psf/BaseExpression.h guf/psf/expressions.cpp \
    guf/psf/expressions.h guf/psf/Base3D.h \
    guf/psf/Polynomial3D.h guf/psf/Polynomial3D.cpp \
    guf/psf/No3D.h guf/psf/No3D.cpp \
    guf/psf/is_plane_dependent.h \
    guf/psf/JointEvaluator.h \
    guf/psf/StandardFunction.h \
    guf/psf/LengthUnit.h guf/psf/ostream.h \
    guf/psf/unit_test.h guf/psf/unit_test.cpp \
    guf/psf/ReferenceEvaluation.h \
    guf/constant_background_fwd.hpp guf/constant_background.hpp

# Makefile magic: The $(a:pat=rep) does suffix replacement and is used with an 
# empty suffix to generate lists
INST_DIR = guf/instantiations/
INST_MODEL = $(INST_DIR:=zhuang_-_) $(INST_DIR:=no3d_-_)
INST_KERNEL = \
    $(INST_MODEL:=free_form_model_-_one_kernel_-_) \
    $(INST_MODEL:=fixed_form_model_-_one_kernel_-_) \
    $(INST_MODEL:=fixed_form_model_-_two_kernel_-_)

INST_NUMTYPE = $(INST_KERNEL:=float_-_) $(INST_KERNEL:=double_-_)
INST_TAGS_UNSEP = $(INST_NUMTYPE:=joint_8) $(INST_NUMTYPE:=disjoint_14) 
if COMPILE_SPECIALIZED_FITTERS
INST_TAGS_UNSEP += $(INST_NUMTYPE:=disjoint_8) \
    $(INST_NUMTYPE:=disjoint_10) $(INST_NUMTYPE:=disjoint_12) \
    $(INST_NUMTYPE:=disjoint_16) $(INST_NUMTYPE:=disjoint_18) \
    $(INST_NUMTYPE:=disjoint_20) 
endif

INST_TAGS = $(INST_TAGS_UNSEP:=_-_)
INSTANTIATIONS_UNCPP = \
    $(INST_KERNEL:=FunctionRepository) $(INST_KERNEL:=ModelledFitter) \
    $(INST_KERNEL:=Moveable) $(INST_TAGS:=PlaneFunction) 
INSTANTIATIONS=$(INSTANTIATIONS_UNCPP:=.cpp)

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    guf/instantiations/one_kernel.h \
    guf/instantiations/two_kernel.h \
    guf/instantiations/no3d.h \
    guf/instantiations/zhuang.h \
    guf/instantiations/fixed_form_model.h \
    guf/instantiations/free_form_model.h \
    guf/instantiations/float.h \
    guf/instantiations/double.h \
    guf/instantiations/disjoint.h \
    guf/instantiations/joint_8.h \
    guf/instantiations/FunctionRepository.h \
    guf/instantiations/PlaneFunction.h \
    guf/instantiations/ModelledFitter.h \
    guf/instantiations/Moveable.h \
    guf/instantiations/Fitter.cpp
nodist_dstorm_@DSTORM_LIB_VERSION@_SOURCES = $(INSTANTIATIONS)
CLEANFILES += $(INSTANTIATIONS)

$(INSTANTIATIONS) : 
	echo $@  \
	    | grep -v '/float_-_.*_-_disjoint_1[048]_-_' \
	    | sed -e 's:^$(INST_DIR)::' -e 's/.cpp$$//' -e 's/_-_/\t/g' \
	    | awk '{ for (i = 1; i <= NF; ++i) print "#include \"" $$i ".h\""; }' \
	    | sed -e 's:#include "disjoint_\([0-9]*\).h":static const int DisjointWidth = \1;\n#include "disjoint.h":' \
	    | sed -e 's:#include ":&guf/instantiations/:' > $@

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    nonlinfit/unit_test.cpp \
    nonlinfit/sum/unit_test.cpp \
    nonlinfit/terminators/unit_test.cpp \
    guf/unit_tests.cpp \
    dejagnu.h unit_tests.h unit_tests.cpp

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    input_simulation/plugin.cpp input_simulation/plugin.h \
    input_simulation/NoiseSource.h input_simulation/NoiseSource.cpp \
    input_simulation/NoiseGenerator.cpp \
    input_simulation/Fluorophore.cpp \
    input_simulation/pixelatedBessel.h input_simulation/pixelatedBessel.cpp \
    input_simulation/FluorophoreDistributions_Random.cpp \
    input_simulation/FluorophoreDistributions_Lattice.cpp \
    input_simulation/FluorophoreDistributions_Lines.cpp \
    input_simulation/FluorophoreDistribution.h \
    input_simulation/FluorophoreDistributions.h \
    input_simulation/NoiseGenerator.h input_simulation/Fluorophore.h 

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    locprec/plugin.cpp locprec/plugin.h \
    locprec/foreach.h \
    locprec/Counter.h locprec/Noise.h locprec/SpotMeter.h \
    locprec/RegionSegmenter.cpp \
    locprec/EmissionTracker.h locprec/EmissionTracker.cpp \
    locprec/SpotMeter.cpp \
    locprec/RegionSegmenter.h \
    locprec/relate.h \
    locprec/PrecisionEstimator.cpp locprec/PrecisionEstimator.h \
    locprec/RegionOfInterest.cpp locprec/RegionOfInterest.h \
    locprec/SourceValuePrinter.cpp locprec/SourceValuePrinter.h \
    locprec/KalmanTrace.h locprec/DensityProfile.h \
    locprec/DistanceHistogram.h locprec/DistanceHistogram.cpp \
    locprec/RipleyK.h locprec/RipleyK.cpp \
    locprec/VarianceEstimator.h locprec/VarianceEstimator.cpp \
    locprec/unit_tests.cpp 

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    AndorCamera/CameraConnection.h AndorCamera/CameraConnection.cpp \
    AndorCamera/InputChainLink_decl.h AndorCamera/InputChainLink.h AndorCamera/InputChainLink.cpp \
    AndorCamera/LiveView.h AndorCamera/LiveView.cpp \
    AndorCamera/AndorDirect.h AndorCamera/AndorDirect.cpp AndorCamera/AndorDirect_iterator.cpp \
    AndorCamera/ViewportSelector.h AndorCamera/ViewportSelector.cpp \
    AndorCamera/plugin.h AndorCamera/plugin.cpp AndorCamera/AndorDirect_decl.h

dstorm_@DSTORM_LIB_VERSION@_SOURCES += \
    tiff/TIFF.h tiff/TIFF.cpp \
    tiff/OpenFile.h tiff/TIFF_OpenFile.cpp \
    tiff/RawImageFile.cpp tiff/RawImageFile.h \
    tiff/TIFFOperation.h tiff/TIFFOperation.cpp \
    tiff/augment_config.h tiff/augment_config.cpp

align_biplane_rapidstorm@DSTORM_LIB_VERSION@_SOURCES = alignment_fitter.cpp
align_biplane_rapidstorm@DSTORM_LIB_VERSION@_CPPFLAGS = $(gsl_CFLAGS)
align_biplane_rapidstorm@DSTORM_LIB_VERSION@_LDADD = librapidstormfile.la libdStorm.la $(gsl_LIBS) 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = librapidstorm.pc librapidstormfile.pc
