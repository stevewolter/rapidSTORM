ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign subdir-objects

TESTS = Pixel_test
check_PROGRAMS = Pixel_test

Pixel_test_SOURCES = Pixel_test.cpp

lib_LTLIBRARIES = libdStorm.la
noinst_LTLIBRARIES = libhelpers.la libinput.la liblocfile.la libengine.la liboutput.la liboutputs.la

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/..

if HAVE_WINDRES
    RESOURCE_FILE = windows_resources.rc
    RESOURCE_OBJECT = windows_resources.o
else
    RESOURCE_FILE =
endif

.rc.o : 
	$(WINDRES) -I . '$<' '$@'
windows_resources.o : config.h

libhelpers_la_SOURCES = \
    helpers/thread.cpp helpers/DisplayManager.cpp helpers/dilation.cpp \
    helpers/BlockingThreadRegistry.cpp helpers/OutOfMemory.cpp
libinput_la_SOURCES = \
    input/Config.cpp input/Method.cpp \
    input/ImageSource.cpp \
    input/Source.cpp input/AutoInputMethod.h \
    input/BasenameWatcher.cpp \
    input/FileBasedMethod.cpp \
    input/ROIFilter.h input/ResolutionSetter.h \
    input/Mask.h input/Mask.cpp
liblocfile_la_SOURCES = \
    localization_file/fields.cpp localization_file/reader.cpp localization_file/known_fields.cpp
liblocfile_la_LIBADD = -lberghen_xmlParser
libengine_la_SOURCES = \
    engine/CandidateTree.cpp engine/Config.cpp engine/SpotFinder.cpp engine/SpotFitterFactory.cpp engine/Image.cpp
liboutput_la_SOURCES = \
    output/debug.h \
    output/Basename.cpp \
    output/BasenameAdjustedFileEntry.cpp \
    output/Config.cpp \
    output/FilterSource.cpp \
    output/OutputSource.cpp output/Trace.cpp \
    output/Traits.cpp \
    output/Localizations.cpp \
    output/TraceReducers.cpp \
    output/SourceFactory.cpp 
liboutputs_la_SOURCES = \
    outputs/BinnedLocalizations.cpp outputs/LocalizationList.cpp outputs/TraceFilter.cpp outputs/Crankshaft.cpp \
    outputs/BinnedLocalizations_impl.h outputs/NullOutput.cpp

library_includedir = $(includedir)/dStorm
nobase_library_include_HEADERS = \
    ModuleInterface.h Localization_decl.h Localization.h \
    SizeTraits.h SizeTraits_decl.h \
    units_Eigen_traits.h unit_matrix_operators.h \
    matrix_operators.h Pixel_decl.h Pixel.h error_handler.h  \
    BaseImage_decl.h BaseImage.h \
    Image_decl.h Image.h Image_impl.h Image_iterator.h \
    ImageTraits_decl.h ImageTraits.h ImageTraits_impl.h \
    Job.h JobMaster.h UnitEntries.h log.h check_plugin.h \
    Config_decl.h Config.h 
outputs_library_includedir = $(includedir)/dStorm/outputs
outputs_library_include_HEADERS = \
    outputs/BinnedLocalizations.h outputs/LocalizationList.h outputs/TraceFilter.h outputs/Crankshaft.h outputs/NullOutput.h outputs/BinnedLocalizations_converter.h

output_library_includedir = $(includedir)/dStorm/output
output_library_include_HEADERS = \
    output/Basename_decl.h output/Basename.h \
    output/BasenameAdjustedFileEntry_decl.h output/BasenameAdjustedFileEntry.h \
    output/Capabilities.h \
    output/Config.h \
    output/FileOutputBuilder.h \
    output/FilterBuilder.h output/FilterBuilder_impl.h \
    output/FilterSource.h \
    output/Localizations.h \
    output/OutputBuilder.h output/OutputBuilder_impl.h \
    output/SourceFactory_decl.h output/SourceFactory.h \
    output/Output.h \
    output/OutputSource.h \
    output/ResultRepeater.h \
    output/Trace_decl.h output/Trace.h \
    output/Traits_decl.h output/Traits.h \
    output/TraceReducer.h

engine_library_includedir = $(includedir)/dStorm/engine
engine_library_include_HEADERS = \
    engine/Candidate_decl.h engine/Candidate.h \
    engine/CandidateTree_decl.h engine/CandidateTree.h \
    engine/Config_decl.h engine/Config.h \
    engine/Image_decl.h engine/Image.h \
    engine/Input_decl.h engine/Input.h \
    engine/Spot_decl.h engine/Spot.h \
    engine/SpotFinder_decl.h engine/SpotFinder.h \
    engine/JobInfo_decl.h engine/JobInfo.h \
    engine/SpotFitter_decl.h engine/SpotFitter.h \
    engine/SpotFitterFactory.h engine/SpotFitterBuilder.h
locfile_library_includedir = $(includedir)/dStorm/localization_file
locfile_library_include_HEADERS = \
	localization_file/fields_decl.h localization_file/fields.h localization_file/fields_impl.h \
	localization_file/known_fields_decl.h localization_file/known_fields.h \
	localization_file/reader.h
input_library_includedir = $(includedir)/dStorm/input
input_library_include_HEADERS = \
	input/AutoInputMethod.h \
	input/BasenameWatcher_decl.h input/BasenameWatcher.h \
	input/Buffer_decl.h input/Buffer.h input/Buffer_impl.h input/Buffer_impl_iterator.h \
	input/Config_decl.h input/Config.h \
	input/Drain_decl.h input/Drain.h \
	input/FileBasedMethod.h input/FileBasedMethod_impl.h \
	input/Method_decl.h input/Method.h \
	input/Slot_decl.h input/Slot.h input/Slot_impl.h \
	input/Source_decl.h input/Source.h input/Source_impl.h \
	input/Traits.h  \
	input/LocalizationTraits_decl.h input/LocalizationTraits.h \
	input/InputFilter.h input/InputFilter_impl.h
helpers_library_includedir = $(includedir)/dStorm/helpers
helpers_library_include_HEADERS = \
    helpers/thread.h helpers/Runnables.h helpers/dilation.h helpers/Variance.h \
    helpers/DisplayDataSource.h helpers/DisplayDataSource_impl.h \
    helpers/DisplayManager.h helpers/errors.h helpers/MayBeASignal.h \
    helpers/BlockingThreadRegistry.h helpers/dilation_impl.h helpers/OutOfMemory.h \
    helpers/exception.h
datacpp_library_includedir = $(includedir)/dStorm/data-c++
datacpp_library_include_HEADERS = \
    data-c++/Vector.h data-c++/MergingTree.h \
    data-c++/VectorList.h data-c++/impl/MergingTree.cpp \
    data-c++/AutoList.h data-c++/Traits.h data-c++/List.h
image_library_includedir = $(includedir)/dStorm/image
image_library_include_HEADERS = \
    image/constructors.h image/convert.h image/iterator.h image/minmax.h
units_library_includedir = $(includedir)/dStorm/units
units_library_include_HEADERS = \
	units/amplitude.h units/frame_count.h \
	units/frame_rate.h units/megafrequency.h \
	units/microtime.h units/nanolength.h \
	units/nanoresolution.h units/prefixes.h
unitentry_library_includedir = $(includedir)/dStorm/UnitEntries
unitentry_library_include_HEADERS = \
	UnitEntries/FrameEntry.h UnitEntries/Hertz.h \
	UnitEntries/Nanometre.h UnitEntries/PixelEntry.h \
	UnitEntries/PixelSize.h UnitEntries/TemperatureEntry.h \
	UnitEntries/TimeEntry.h UnitEntries/ADC.h

libdStorm_la_SOURCES = Localization.cpp debug.h debug_print.h \
    DeferredError.h DeferredError_Impl.h DeferredError.cpp \
    error_handler.cpp JobMaster.cpp $(RESOURCE_FILE) UnitEntries.cpp Config.cpp \
    units/units.cpp ImageTraits.cpp
libdStorm_la_LIBADD = liboutputs.la liboutput.la \
    libengine.la liblocfile.la \
    libinput.la libhelpers.la -lsimparm
libdStorm_la_LDFLAGS = -no-undefined -version-info @LIBDSTORM_CURRENT@:@LIBDSTORM_REVISION@:@LIBDSTORM_AGE@ -Xlinker $(RESOURCE_OBJECT)
libdStorm_la_DEPENDENCIES = $(RESOURCE_OBJECT) $(libdStorm_la_LIBADD)
