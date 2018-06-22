if BUILD_DOCUMENTATION
    MAYBE_DOC = doc
endif
SUBDIRS = $(MAYBE_DOC)

ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = subdir-objects dejagnu

AM_CPPFLAGS = -DBOOST_TEST_DYN_LINK
# TODO: This flag is only needed for old Eigen3 versions up to the fix of Eigen bug 477
AM_CXXFLAGS = -Wno-enum-compare -std=c++0x -Wno-deprecated-declarations -Wnon-virtual-dtor
# Workaround for https://github.com/google/protobuf/issues/1797
AM_CXXFLAGS += -Wno-misleading-indentation

# Doxygen support
@DX_RULES@
MOSTLYCLEANFILES = $(DX_CLEANFILES)

applicationsdir=$(datadir)/applications
pkgconfigdir = $(libdir)/pkgconfig

bin_PROGRAMS = rapidstorm 
check_PROGRAMS = rapidstorm
applications_DATA=rapidstorm.desktop
doc_DATA = $(srcdir)/README 
pkgdata_DATA = dstorm-config.txt

AM_RUNTESTFLAGS = EXEEXT=$(EXEEXT)
if REMOTE_TESTING
    AM_RUNTESTFLAGS += --target_board $(TELNET_HOST)
endif

if INTEGRATION_TESTING
    THE_DEJATOOL = dstorm
else
    THE_DEJATOOL = no-integration-tests
endif
DEJATOOL = $(THE_DEJATOOL)

EXTRA_DIST = windows_resources.rc \
    Doxyfile.in gaussian_psf/make_psf.sh \
    dstorm-config.txt license_note_for_user

CLEANFILES = rapidSTORM.jar \
	rapidstorm.desktop rapidstorm \
	installation-directory.h

always-force-remake :

installation-directory.h : always-force-remake
	echo '#define CONFIG_FILE_DIR "$(pkgdatadir)"' > $@.tmp
	echo '#define PREFIX "$(prefix)"' >> $@.tmp
	diff --brief $@ $@.tmp || mv $@.tmp $@
	rm -f $@.tmp

PROTO_SOURCE_FILES = tsf/TSFProto.proto
PROTO_GENERATED = $(PROTO_SOURCE_FILES:.proto=.pb.cc) $(PROTO_SOURCE_FILES:.proto=.pb.h)
dist_noinst_DATA = $(PROTO_SOURCE_FILES)
BUILT_SOURCES = installation-directory.h
CLEANFILES += $(PROTO_GENERATED)
if HAVE_PROTOBUF
BUILT_SOURCES += $(PROTO_GENERATED)
endif

all-local : doxygen-doc

clean-local :
	-rm -fr test-temp-dir

TESTS = dstorm-unit-tests.sh
EXTRA_DIST += dstorm-unit-tests.sh

if HAVE_WINDRES
    RESOURCE_FILE = windows_resources.rc
    RESOURCE_OBJECT = windows_resources.o
    LINKER_RESOURCE = -Xlinker
else
    RESOURCE_FILE =
endif
EXTRA_DIST += Microscope.ico

if IS_AMD64_BUILD
    # The wxWidgets CPU definition for 64 bit is buggy, breaking the manifest
    # file selection code and subsequently producing unstartable executables.
    # We fix it by defining the symbol that *should* have been defined in the
    # selection code.
    CPU_DEF = -DWX_CPU_AMD64=1
else
    CPU_DEF =
endif

.rc.o : 
	$(WX_RESCOMP) $(CPU_DEF) -I . '$<' '$@'

rapidstorm_SOURCES = \
    installation-directory.h \
    LengthUnit.h \
    polynomial_3d.h \
    InsertionPlace.h \
    Localization_decl.h Localization.h \
    Pixel.h \
    DataSetTraits.h \
    Job.h UnitEntries.h \
    Config.h \
    Engine.h types/samplepos.h types/fluorophore.h \
    namespaces.h \
    unit_interval.h \
    stack_realign.h \
    traits/Projection.h traits/ProjectionConfig.h \
    traits/ProjectionFactory.h traits/ScaledProjection.h \
    traits/optics.h traits/image_resolution.h traits/optics_config.h \
    threed_info/types.h \
    threed_info/DepthInfo.h \
    threed_info/Config.h \
    threed_info/look_up_sigma_diff.h \
    localization/Field.h localization/Traits_decl.h localization/Traits.h \
    localization/record_decl.h localization/record.h\
    localization/Fields.h localization/Fields.cpp \
    output/Basename_decl.h output/Basename.h \
    output/BasenameAdjustedFileEntry_decl.h output/BasenameAdjustedFileEntry.h \
    output/Capabilities.h \
    output/FileOutputBuilder.h \
    output/FilterBuilder.h output/FilterBuilder_impl.h \
    output/FilterSource.h \
    output/OutputBuilder.h output/OutputBuilder_impl.h \
    output/SourceFactory_decl.h output/SourceFactory.h \
    output/Output_decl.h output/Output.h output/Filter.h \
    output/OutputSource.h \
    output/Traits_decl.h output/Traits.h \
    output/TraceReducer.h \
    output/LocalizedImage_decl.h output/LocalizedImage.h output/LocalizedImage_traits.h \
    engine/Candidate_decl.h engine/Candidate.h \
    engine/CandidateTree_decl.h engine/CandidateTree.h \
    engine/Image_decl.h engine/Image.h \
    engine/Input_decl.h engine/Input.h \
    engine/Spot.h \
    engine/SpotFinder_decl.h engine/SpotFinder.h \
    engine/SpotFinderBuilder.h \
    engine/JobInfo_decl.h engine/JobInfo.h \
    engine/SpotFitter_decl.h engine/SpotFitter.h \
    engine/FitPosition.h \
    engine/SpotFitterFactory_decl.h engine/SpotFitterFactory.h \
    engine/InputTraits.h engine/InputPlane.h \
    engine/FitJudger.h engine/FitJudgerFactory.h engine/FitJudger.cpp \
    input/AdapterSource.h \
    input/Choice.h \
    input/fwd.h input/Link.h \
    input/Forwarder.h \
    input/DefaultFilterTypes.h \
    input/MetaInfo.h \
    input/Source.h \
    input/Traits.h  \
    input/InputMutex.h \
    input/FileInput.h \
    helpers/thread.h \
    helpers/default_on_copy.h \
    helpers/nocopy_ptr.hpp \
    helpers/OutOfMemory.h \
    helpers/clone_ptr.hpp helpers/make_boost_clone.h \
    helpers/back_inserter.h \
    image/Image.h image/Image.hpp \
    image/fwd.h image/MetaInfo.h image/MetaInfo.hpp \
    image/constructors.h image/convert.h image/iterator.h image/minmax.h image/slice.h \
    image/mirror.h image/crop.h image/extend.h \
    image/dilation.h image/dilation_impl.h image/contains.h \
    image/find_by_offset.hpp image/normalize.h \
    image/corners.h \
    image/morphological_reconstruction.h \
    image/morphological_reconstruction.hpp \
    image/Box.h image/Box.hpp \
    image/unit_tests.cpp \
    units/icl.h \
    units/amplitude.h units/frame_count.h \
    units/megafrequency.h \
    units/microtime.h units/nanolength.h \
    units/microlength.h units/permicrolength.h \
    units/nanoresolution.h \
    units/camera_response.h \
    UnitEntries/FrameEntry.h UnitEntries/Hertz.h \
    UnitEntries/Nanometre.h UnitEntries/PixelEntry.h \
    UnitEntries/PixelSize.h UnitEntries/TemperatureEntry.h \
    UnitEntries/TimeEntry.h UnitEntries/ADC.h \
    signals/UseSpotFinder.h signals/UseSpotFitter.h \
    signals/InputFileNameChange.h signals/ResolutionChange.h signals/BasenameChange.h \
    display/fwd.h \
    display/DataSource.h display/display_normalized.hpp \
    display/Manager.h \
    display/SharedDataSource.h display/SharedDataSource.cpp \
    display/store_image.h display/store_image.cpp \
    Direction.h \
    threed_info/Polynomial3D.h \
    threed_info/No3D.h \
    threed_info/Lens3D.h \
    threed_info/Spline3D.h \
    threed_info/fwd.h

rapidstorm_SOURCES += \
    localization_file/converter.h \
    localization_file/field.h localization_file/field_decl.h \
    localization_file/fields.cpp \
    localization_file/localization_field.h \
    localization_file/localization_field_impl.h \
    localization_file/reader.cpp localization_file/reader.h \
    localization_file/writer.h localization_file/writer.cpp  \
    localization_file/unknown_field.h 

rapidstorm_SOURCES += \
    Localization.cpp \
    UnitEntries.cpp \
    DataSetTraits.cpp units/units.cpp image/MetaInfo.cpp \
    localization/Field.cpp \
    localization/Traits.cpp \
    image/instantiate.cpp \
    traits/optics.cpp traits/optics_config.cpp \
    traits/unit_tests.h traits/unit_tests.cpp \
    traits/Projection.cpp \
    traits/AffineProjection.h traits/AffineProjection.cpp \
    traits/ScaledProjection.cpp \
    traits/SupportPointProjection.cpp \
    localization/record.cpp \
    helpers/thread.cpp display/Manager.cpp image/dilation.cpp \
    image/Box.cpp \
    helpers/OutOfMemory.cpp \
    input/Source.cpp \
    input/MetaInfo.cpp \
    input/InputMutex.cpp \
    input/Link.cpp input/Choice.cpp \
    input/Forwarder.cpp \
    input/Method.hpp \
    engine/CandidateTree.cpp engine/SpotFinder.cpp engine/SpotFitterFactory.cpp engine/Image.cpp \
    engine/JobInfo.cpp engine/InputTraits.cpp \
    engine/InputPlane.cpp \
    output/Output.cpp output/LocalizedImage.cpp \
    output/Basename.cpp \
    output/BasenameAdjustedFileEntry.cpp \
    output/Config.cpp \
    output/FilterSource.cpp \
    output/OutputSource.cpp \
    output/Traits.cpp \
    output/TraceReducers.cpp \
    output/SourceFactory.cpp  \
    output/LocalizedImage_traits.cpp \
    output/Filter.cpp \
    Pixel.cpp \
    image/slice.cpp image/iterator.cpp \
    image/morphological_reconstruction.cpp \
    threed_info/DepthInfo.cpp \
    threed_info/Polynomial3D.cpp \
    threed_info/Config.cpp \
    threed_info/Spline3D.cpp \
    threed_info/No3D.cpp \
    threed_info/Lens3D.cpp \
    threed_info/look_up_sigma_diff.cpp \
    make_clone_allocator.hpp

rapidstorm_SOURCES += dstorm.cpp installation-directory.h \
    config_file.h config_file.cpp \
    alignment_fitter.h alignment_fitter.cpp

rapidstorm_SOURCES += \
    CommandLine.h CommandLine.cpp \
    ModuleLoader.h ModuleLoader.cpp \
    shell/JobStarter.h shell/JobStarter.cpp \
    shell/JobFactory.h shell/JobMetaFactory.h \
    shell/ReplayJob.h shell/ReplayJob.cpp \
    shell/Shell.cpp \
    InputStream.h InputStream.cpp \
    GUIThread.h GUIThread.cpp \
    debug.h $(RESOURCE_FILE)

rapidstorm_SOURCES += \
    job/Car.cpp job/Car.h \
    job/Config.h job/Config.cpp \
    job/Queue.h job/Queue.cpp \
    job/Run.h job/Run.cpp \
    job/Control.h job/Control.cpp \
    job/OutputTreeRoot.h job/OutputTreeRoot.cpp

rapidstorm_SOURCES += \
    output/Config.h

rapidstorm_SOURCES += \
    inputs/inputs.h \
    inputs/FileMethod.h inputs/FileMethod.cpp \
    inputs/InputMethods.h inputs/InputMethods.cpp \
    inputs/InputBase.cpp inputs/InputBase.h \
    inputs/InsertionPoint.cpp inputs/InsertionPoint.h \
    inputs/join.h inputs/join.cpp \
    inputs/join/spatial.h inputs/join/spatial.cpp \
    inputs/join/temporal.h inputs/join/temporal.cpp \
    inputs/Splitter.h inputs/Splitter.cpp \
    inputs/YMirror.h inputs/YMirror.cpp \
    inputs/ResolutionSetter.h \
    inputs/ResolutionSetter.h inputs/ResolutionSetter.cpp \
    inputs/SampleInfo.h inputs/SampleInfo.cpp \
    inputs/ROIFilter.h inputs/ROIFilter.cpp \
    inputs/PlaneFilter.h inputs/PlaneFilter.cpp \
    inputs/Basename.h inputs/Basename.cpp \
    inputs/Buffer.h inputs/Buffer.cpp \
    inputs/LocalizationFile.h inputs/LocalizationFile.cpp  \
    inputs/WarnAboutLocalizationFile.h inputs/WarnAboutLocalizationFile.cpp \
    inputs/MedianFilter.h inputs/MedianFilter.cpp \
    inputs/MedianFilter_test.h inputs/MedianFilter_test.cpp \
    inputs/unit_tests.h inputs/unit_tests.cpp

rapidstorm_SOURCES += \
	noop_engine/ChainLink.cpp noop_engine/ChainLink_decl.h \
	noop_engine/ChainLink.h noop_engine/Engine.cpp noop_engine/Engine.h

rapidstorm_SOURCES += \
    engine/Config.cpp engine/Config_decl.h engine/Config.h \
    engine/Engine.cpp \
    engine/EngineDebug.h engine/Engine.h \
    engine/ChainLink_decl.h engine/ChainLink.h engine/ChainLink.cpp \
    engine/PlaneFlattener.h engine/PlaneFlattener.cpp

rapidstorm_SOURCES += \
	    engine_stm/LocalizationBuncher.cpp engine_stm/LocalizationBuncher.h \
	    engine_stm/ChainLink.h engine_stm/ChainLink.cpp

rapidstorm_SOURCES += \
    expression/Config.h  expression/Config_decl.h \
    expression/Source.h  expression/Source_decl.h \
    expression/Evaluator.h expression/Simplifier.h expression/UnitChecker.h \
    expression/localization_variable.cpp expression/localization_variable_decl.h \
    expression/SIPrefixes.h \
    expression/Variable.h \
    expression/DynamicQuantity.h \
    expression/Parser.h expression/Parser.cpp expression/Parser_test.cpp \
    expression/QuantityDynamizer.h expression/QuantityDynamizer.hpp \
    expression/tokens.cpp expression/tokens_decl.h expression/tokens.h \
    expression/types.cpp expression/types.h \
    expression/UnitTable.cpp expression/UnitTable.h expression/UnitTable_impl.h \
    expression/Config.cpp expression/Source.cpp \
    expression/CommandLine.h expression/CommandLine.cpp \
    expression/LValue.h expression/LValue.cpp \
    expression/SimpleFilters.h expression/SimpleFilters.cpp \
    expression/VariableLValue.h expression/VariableLValue.cpp \
    expression/Filter.h expression/Filter.cpp \
    expression/unit_test.cpp

rapidstorm_SOURCES += \
    outputs/TraceFilter.h outputs/TraceFilter.cpp \
    outputs/DriftRemover.cpp outputs/DriftRemover.h \
    outputs/AverageImage.cpp outputs/AverageImage.h outputs/BasicTransmissions.cpp \
    outputs/LocalizationCounter.cpp outputs/LocalizationCounter.h \
    outputs/ProgressMeter.cpp \
    outputs/ProgressMeter.h \
    outputs/Slicer.cpp outputs/Slicer.h \
    outputs/BasicTransmissions.h \
    outputs/MemoryCache.cpp outputs/MemoryCache.h \
    outputs/MemoryCache_Cache.h outputs/MemoryCache_Cache.cpp \
    outputs/SigmaDiff3D.h outputs/SigmaDiff3D.cpp \
    outputs/LinearAlignment.h outputs/LinearAlignment.cpp \
    outputs/Crankshaft.h outputs/Crankshaft.cpp \
    outputs/RegionOfInterest.h outputs/RegionOfInterest.cpp \
    outputs/SpotMeter.h outputs/SpotMeter.cpp \
    outputs/RegionSegmenter.h outputs/RegionSegmenter.cpp \
    outputs/PrecisionEstimator.cpp outputs/PrecisionEstimator.h \
    outputs/VarianceEstimator.h outputs/VarianceEstimator.cpp \
    outputs/NullOutput.h 


rapidstorm_SOURCES += \
    calibrate_3d/fwd.h \
    calibrate_3d/Output.h calibrate_3d/Output.cpp \
    calibrate_3d/Config.h calibrate_3d/Config.cpp \
    calibrate_3d/ParameterLinearizer.h calibrate_3d/ParameterLinearizer.cpp \
    calibrate_3d/FormCalibrationConfig.h calibrate_3d/FormCalibrationConfig.cpp \
    calibrate_3d/ZTruth.h calibrate_3d/ZTruth.cpp \
    calibrate_3d/ZTruthConfig.h calibrate_3d/ZTruthConfig.cpp \
    calibrate_3d/constant_parameter.hpp \
    calibrate_3d/SigmaCurve.cpp

rapidstorm_SOURCES += \
    spotFinders/averageSmooth.cpp \
    spotFinders/averageSmooth.h \
    spotFinders/ErosionSmoother.cpp \
    spotFinders/GaussSmoothing.cpp \
    spotFinders/MedianSmoother.cpp \
    spotFinders/Spalttiefpass.cpp \
    spotFinders/Spaltbandpass.cpp \
    spotFinders/Fillhole.h spotFinders/Fillhole.cpp \
    spotFinders/spotFinders.cpp spotFinders/spotFinders.h

rapidstorm_SOURCES += \
    viewer/fwd.h \
    viewer/DensityMap.cpp \
    viewer/Image.h viewer/Image.cpp \
    viewer/Display.h viewer/Display.cpp \
    viewer/ImageDiscretizer.h \
	viewer/ImageDiscretizer.hpp viewer/ImageDiscretizer.cpp \
    viewer/Config.h viewer/Config.cpp \
    viewer/DensityMapConfig.h viewer/DensityMapConfig.cpp \
    viewer/Backend.h viewer/Backend.cpp \
    viewer/Viewer.cpp viewer/Viewer.h \
    viewer/Publisher.h viewer/HighDepth.h \
    viewer/LiveBackend.h viewer/LiveBackend.cpp \
    viewer/LiveCache.h viewer/LiveCache.cpp \
    viewer/TerminalBackend.h viewer/TerminalBackend.cpp \
    viewer/TerminalCache.h viewer/TerminalCache.cpp \
    viewer/Status.h viewer/Status.cpp \
    viewer/ImageDiscretizer_converter.h \
    viewer/plugin.h viewer/plugin.cpp \
    viewer/ColourScheme.h viewer/ColourSchemeFactory.h  \
    viewer/colour_schemes/decl.h \
    viewer/colour_schemes/base.cpp viewer/colour_schemes/base.h \
    viewer/colour_schemes/hot.cpp viewer/colour_schemes/greyscale.cpp \
    viewer/colour_schemes/colored.cpp viewer/colour_schemes/coordinate.cpp \
    viewer/colour_schemes/HueSaturationMixer.h viewer/colour_schemes/HueSaturationMixer.cpp

rapidstorm_SOURCES += \
    test-plugin/plugin.cpp test-plugin/plugin.h \
    test-plugin/Exception.h test-plugin/Delayer.h \
    test-plugin/BasenamePrinter.h test-plugin/BasenamePrinter.cpp \
    test-plugin/DummyFileInput.h test-plugin/DummyFileInput.cpp \
    test-plugin/Verbose.h \
    test-plugin/RepeatTrigger.h test-plugin/SmoothedImageSave.h \
    test-plugin/xenophon.h test-plugin/xenophon.cpp \
    test-plugin/cpu_time.h test-plugin/cpu_time.cpp

rapidstorm_SOURCES += \
    estimate_psf_form/Output.h estimate_psf_form/Output.cpp \
    estimate_psf_form/Fitter.h estimate_psf_form/Fitter.cpp \
    estimate_psf_form/Config.h estimate_psf_form/Config.cpp \
    estimate_psf_form/decl.h estimate_psf_form/Tile.h \
    estimate_psf_form/GUI.h estimate_psf_form/GUI.cpp \
    estimate_psf_form/Input.h estimate_psf_form/Input.cpp \
    estimate_psf_form/LocalizationValueFinder.h estimate_psf_form/LocalizationValueFinder.cpp


rapidstorm_SOURCES += \
    nonlinfit/AbstractFunction.h \
    nonlinfit/access_parameters.hpp \
    nonlinfit/append.h \
    nonlinfit/BoundFunction.h nonlinfit/BoundFunction.hpp \
    nonlinfit/DerivationSummand.h \
    nonlinfit/Evaluation.h \
    nonlinfit/Evaluator.h \
    nonlinfit/derive_by.hpp \
    nonlinfit/Bind.h \
    nonlinfit/FunctionConverter.h \
    nonlinfit/VectorPosition.h nonlinfit/VectorPosition.hpp \
    nonlinfit/fwd.h \
    nonlinfit/index_of.h \
    nonlinfit/make_bitset.h \
    nonlinfit/make_functor.hpp \
    nonlinfit/parameter_is_negligible.hpp \
    nonlinfit/Lambda.h \
    nonlinfit/Terminator.h \
    nonlinfit/TermParameter.h \
    nonlinfit/Evaluation.hpp \
    nonlinfit/Xs.h \
    nonlinfit/InvalidPositionError.h \
    nonlinfit/functions/Constant.h \
    nonlinfit/functions/Polynom.h \
    nonlinfit/functions/Zero.h \
    nonlinfit/levmar/Config.h \
    nonlinfit/levmar/Fitter.h \
    nonlinfit/levmar/Fitter.cpp \
    nonlinfit/levmar/exceptions.h \
    nonlinfit/plane/fwd.h \
    nonlinfit/plane/check_evaluator.hpp \
    nonlinfit/plane/Distance.h nonlinfit/plane/Distance.hpp \
    nonlinfit/plane/Joint.h \
    nonlinfit/plane/Disjoint.h nonlinfit/plane/Disjoint.hpp \
    nonlinfit/plane/JointData.h \
    nonlinfit/plane/DisjointData.h nonlinfit/plane/DisjointData.hpp \
    nonlinfit/plane/DisjointData.cpp \
    nonlinfit/plane/DataFacade.h nonlinfit/plane/DataFacade.hpp \
    nonlinfit/plane/GenericData.h \
    nonlinfit/plane/DataPoint.h \
    nonlinfit/plane/JointData.hpp \
    nonlinfit/plane/Jacobian.h \
    nonlinfit/terminators/StepLimit.h \
    nonlinfit/sum/AbstractFunction.h \
    nonlinfit/sum/AbstractFunction.cpp \
    nonlinfit/sum/VariableMap.h \
    nonlinfit/sum/VariableMap.hpp \
    nonlinfit/sum/VariableMap.cpp

rapidstorm_SOURCES += \
    fit_window/Config.h fit_window/Config.cpp \
    fit_window/Plane.h fit_window/Plane.cpp \
    fit_window/fit_position_out_of_range.h fit_window/fit_position_out_of_range.cpp \
    fit_window/Spot.h \
    fit_window/Optics.h fit_window/Optics.cpp \
    fit_window/FitWindowCutter.h fit_window/FitWindowCutter.cpp \
    fit_window/chunkify.h fit_window/chunkify.hpp \
    fit_window/unit_tests.h fit_window/unit_tests.cpp

rapidstorm_SOURCES += \
    guf/fitter.h guf/fitter.cpp \
    guf/EvaluationTags.h \
    guf/Config.cpp guf/Config_decl.h guf/Config.h \
    guf/create_evaluator.h guf/create_evaluator.hpp \
    guf/FunctionRepository.h guf/FunctionRepository.hpp \
    guf/Factory.cpp guf/Factory.h \
    guf/MultiKernelModel.h guf/MultiKernelModel.hpp \
    guf/Fitter.cpp guf/Fitter.h \
    guf/InitialValueFinder.h guf/InitialValueFinder.cpp \
    guf/TraitValueFinder.h guf/TraitValueFinder.cpp \
    guf/NaiveFitter.h guf/NaiveFitter.cpp \
    guf/FitFunctionFactory.h guf/FitFunctionFactory.cpp \
    guf/LocalizationChecker.h guf/LocalizationChecker.cpp \
    guf/LocalizationCreator.h guf/LocalizationCreator.cpp \
    guf/KernelCreator.h guf/KernelCreator.cpp \
    guf/PlaneFunction.h guf/PlaneFunction.hpp \
    guf/DistanceMetric.h \
    guf/Spot.h \
    guf/unit_tests.h

rapidstorm_SOURCES += \
    gaussian_psf/fwd.h gaussian_psf/parameters.h \
    gaussian_psf/BaseEvaluator.h gaussian_psf/BaseEvaluator.cpp \
    gaussian_psf/DisjointEvaluator.h \
    gaussian_psf/free_form.h \
    gaussian_psf/fixed_form.h \
    gaussian_psf/BaseExpression.h gaussian_psf/expressions.cpp \
    gaussian_psf/expressions.h gaussian_psf/Base3D.h \
    gaussian_psf/Polynomial3D.h gaussian_psf/Polynomial3D.cpp \
    gaussian_psf/No3D.h gaussian_psf/No3D.cpp \
    gaussian_psf/DepthInfo3D.h gaussian_psf/DepthInfo3D.cpp \
    gaussian_psf/is_plane_dependent.h \
    gaussian_psf/JointEvaluator.h \
    gaussian_psf/unit_test.h gaussian_psf/unit_test.cpp \
    gaussian_psf/ReferenceEvaluation.h \
    gaussian_psf/mock_model.h gaussian_psf/mock_model.cpp \
    gaussian_psf/check_evaluator.hpp \
    gaussian_psf/polynomial_psf_generated_by_yacas.h

rapidstorm_SOURCES += \
    constant_background/fwd.hpp constant_background/model.hpp

# Makefile magic: The $(a:pat=rep) does suffix replacement and is used with an 
# empty suffix to generate lists
INST_DIR = guf/instantiations/

INST_NUMTYPE = $(INST_DIR:=float_-_) $(INST_DIR:=double_-_)
INST_TAGS_UNSEP = $(INST_NUMTYPE:=joint_8) $(INST_NUMTYPE:=disjoint_14) 
if COMPILE_SPECIALIZED_FITTERS
INST_TAGS_UNSEP += $(INST_NUMTYPE:=disjoint_8) \
    $(INST_NUMTYPE:=disjoint_10) $(INST_NUMTYPE:=disjoint_12) \
    $(INST_NUMTYPE:=disjoint_16) $(INST_NUMTYPE:=disjoint_18) \
    $(INST_NUMTYPE:=disjoint_20) 
endif
INST_TAGS = $(INST_TAGS_UNSEP:=_-_)

INST_MODEL = \
    $(INST_TAGS:=no3d_free_form_model_-_) \
    $(INST_TAGS:=no3d_fixed_form_model_-_) \
    $(INST_TAGS:=depthinfo3d_fixed_form_model_-_) \
    $(INST_TAGS:=constant_background_-_)
INSTANTIATIONS_UNCPP = \
    $(INST_MODEL:=create_evaluator) $(INST_TAGS:=PlaneFunction)
INSTANTIATIONS=$(INSTANTIATIONS_UNCPP:=.cpp)

rapidstorm_SOURCES += \
    guf/instantiations/no3d_free_form_model.h \
    guf/instantiations/no3d_fixed_form_model.h \
    guf/instantiations/depthinfo3d_fixed_form_model.h \
    guf/instantiations/constant_background.h \
    guf/instantiations/float.h \
    guf/instantiations/double.h \
    guf/instantiations/disjoint.h \
    guf/instantiations/joint_8.h \
    guf/instantiations/create_evaluators.h \
    guf/instantiations/PlaneFunction.h
nodist_rapidstorm_SOURCES = $(INSTANTIATIONS)
CLEANFILES += $(INSTANTIATIONS)

$(INSTANTIATIONS) : 
	echo $@  \
	    | grep -v '/float_-_.*_-_disjoint_1[048]_-_' \
	    | $(SED) -e 's:^$(INST_DIR)::' -e 's/.cpp$$//' -e 's/_-_/\t/g' \
	    | awk '{ for (i = 1; i <= NF; ++i) print "#include \"" $$i ".h\""; }' \
	    | $(SED) -e 's:#include "disjoint_\([0-9]*\).h":static const int DisjointWidth = \1;\n#include "disjoint.h":' \
	    | $(SED) -e 's:#include ":&guf/instantiations/:' > $@

rapidstorm_SOURCES += \
    nonlinfit/unit_test.h \
    nonlinfit/unit_test.cpp \
    nonlinfit/sum/unit_test.cpp \
    guf/unit_tests.cpp \
    dejagnu.h unit_tests.h unit_tests.cpp

rapidstorm_SOURCES += \
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

rapidstorm_SOURCES += \
    kalman_filter/fwd.h kalman_filter/EmissionTracker.cpp \
    kalman_filter/KalmanTrace.h \
    kalman_filter/NonlinearDriftEstimator.cpp

rapidstorm_SOURCES += \
    ripley_k/DistanceHistogram.h ripley_k/DistanceHistogram.cpp \
    ripley_k/fwd.h ripley_k/RipleyK.cpp

rapidstorm_SOURCES += \
    tiff/TIFF.h tiff/TIFF.cpp \
    tiff/OpenFile.h tiff/TIFF_OpenFile.cpp \
    tiff/RawImageFile.cpp tiff/RawImageFile.h \
    tiff/TIFFOperation.h tiff/TIFFOperation.cpp \
    tiff/augment_config.h tiff/augment_config.cpp

rapidstorm_SOURCES += \
    simparm/Node.h simparm/Node.cpp \
    simparm/NodeHandle.h \
    simparm/BaseAttribute.h \
    simparm/Choice.h \
    simparm/Attribute.h simparm/Attribute.hpp simparm/Attribute.cpp \
    simparm/BasicEntry.h simparm/BasicEntry.cpp \
    simparm/Group.h simparm/Group.cpp \
    simparm/TabGroup.h simparm/TabGroup.cpp \
    simparm/Entry.h simparm/Entry.hpp simparm/Entry.cpp \
    simparm/Object.h simparm/Object.cpp \
    simparm/TriggerEntry.h simparm/TriggerEntry.cpp \
    simparm/ProgressEntry.h simparm/ProgressEntry.cpp \
    simparm/TreeRoot.h simparm/TreeRoot.cpp \
    simparm/TreeEntry.h simparm/TreeEntry.cpp \
    simparm/FileEntry.h simparm/FileEntry.cpp \
    simparm/Message.h simparm/Message.cpp \
    simparm/ChoiceEntry.h simparm/ChoiceEntry.cpp \
    simparm/ObjectChoice.h simparm/ManagedChoiceEntry.h \
    simparm/Menu.h \
    simparm/iostream.h simparm/typeName.h \
    simparm/Eigen_decl.h simparm/Eigen.h \
    simparm/BoostUnits.h \
    simparm/UserLevel.h simparm/UserLevel.cpp \
    simparm/BoostOptional.h \
    simparm/falls_below.hpp simparm/exceeds.hpp \
    simparm/is_numerical.hpp \
    simparm/GUILabelTable.h simparm/GUILabelTable.cpp \
    simparm/unit_tests.h simparm/unit_tests.cpp

rapidstorm_SOURCES += \
    simparm/text_stream/BackendRoot.h simparm/text_stream/BackendRoot.cpp \
    simparm/text_stream/ChildrenList.h \
    simparm/text_stream/EntryNode.h simparm/text_stream/FrontendNode.h \
    simparm/text_stream/InnerBackendNode.h simparm/text_stream/InnerBackendNode.cpp \
    simparm/text_stream/NodeBackend.cpp simparm/text_stream/NodeBackend.h \
    simparm/text_stream/Node.cpp simparm/text_stream/Node.h \
    simparm/text_stream/RootNode.cpp simparm/text_stream/RootNode.h \
    simparm/text_stream/TabNode.h simparm/text_stream/NoOpNode.h \
    simparm/text_stream/Launcher.h simparm/text_stream/Launcher.cpp \
    simparm/text_stream/unit_tests.h simparm/text_stream/unit_tests.cpp \
    simparm/text_stream/image_window/Window.h simparm/text_stream/image_window/Window.cpp \
    simparm/text_stream/image_window/MainThread.h simparm/text_stream/image_window/MainThread.cpp \
    simparm/text_stream/image_window/md5.h simparm/text_stream/image_window/md5.c

rapidstorm_SOURCES += \
    simparm/wx_ui/App.h simparm/wx_ui/App.cpp \
    simparm/wx_ui/image_window/ImageWindow.h simparm/wx_ui/image_window/ImageWindow.cpp \
    simparm/wx_ui/image_window/Handle.h simparm/wx_ui/image_window/Handle.cpp \
    simparm/wx_ui/image_window/Canvas.h simparm/wx_ui/image_window/Canvas.cpp \
    simparm/wx_ui/image_window/ZoomSlider.h simparm/wx_ui/image_window/ZoomSlider.cpp \
    simparm/wx_ui/image_window/Key.h simparm/wx_ui/image_window/Key.cpp \
    simparm/wx_ui/image_window/helpers.h simparm/wx_ui/image_window/helpers.cpp \
    simparm/wx_ui/image_window/ScaleBar.h simparm/wx_ui/image_window/ScaleBar.cpp \
    simparm/wx_ui/image_window/SizeConvert.h \
    simparm/wx_ui/image_window/create.h \
    simparm/wx_ui/Node.h simparm/wx_ui/Node.cpp \
    simparm/wx_ui/RootNode.h simparm/wx_ui/RootNode.cpp \
    simparm/wx_ui/Launcher.h simparm/wx_ui/Launcher.cpp \
    simparm/wx_ui/TextfieldNode.h simparm/wx_ui/TextfieldNode.cpp \
    simparm/wx_ui/TabNode.h simparm/wx_ui/TabNode.cpp \
    simparm/wx_ui/WindowNode.h simparm/wx_ui/WindowNode.cpp \
    simparm/wx_ui/ScrolledWindowNode.h simparm/wx_ui/ScrolledWindowNode.cpp \
    simparm/wx_ui/ScrolledTabNode.h simparm/wx_ui/ScrolledTabNode.cpp \
    simparm/wx_ui/ChoiceNode.h simparm/wx_ui/ChoiceNode.cpp \
    simparm/wx_ui/OutputChoiceNode.h simparm/wx_ui/OutputChoiceNode.cpp \
    simparm/wx_ui/TriggerNode.h simparm/wx_ui/TriggerNode.cpp \
    simparm/wx_ui/ProgressNode.h simparm/wx_ui/ProgressNode.cpp \
    simparm/wx_ui/AttributeHandle.h \
    simparm/wx_ui/CheckboxNode.h simparm/wx_ui/CheckboxNode.cpp \
    simparm/wx_ui/GroupNode.h simparm/wx_ui/GroupNode.cpp \
    simparm/wx_ui/TreeRoot.h simparm/wx_ui/TreeRoot.cpp \
    simparm/wx_ui/TreePage.h simparm/wx_ui/TreePage.cpp \
    simparm/wx_ui/TreeRepresentation.h simparm/wx_ui/TreeRepresentation.cpp \
    simparm/wx_ui/VisibilityNode.h simparm/wx_ui/VisibilityNode.cpp \
    simparm/wx_ui/VisibilityControl.h simparm/wx_ui/VisibilityControl.cpp \
    simparm/wx_ui/Sizer.h simparm/wx_ui/Sizer.cpp \
    simparm/wx_ui/Window.h simparm/wx_ui/Window.cpp \
    simparm/wx_ui/lambda.h simparm/wx_ui/GUIHandle.h \
    simparm/wx_ui/gui_thread.h simparm/wx_ui/gui_thread.cpp \
    simparm/wx_ui/no_main_window.h simparm/wx_ui/no_main_window.cpp \
    simparm/wx_ui/ProtocolNode.h simparm/wx_ui/ProtocolNode.cpp \
    simparm/wx_ui/stock-hchain-24-broken.xpm simparm/wx_ui/stock-hchain-24.xpm \
    simparm/wx_ui/Microscope_16.xpm simparm/wx_ui/Microscope_24.xpm \
    simparm/wx_ui/Microscope_32.xpm simparm/wx_ui/Microscope_48.xpm \
    simparm/wx_ui/Microscope_64.xpm

rapidstorm_SOURCES += \
    simparm/cmdline_ui/ChoiceNode.h simparm/cmdline_ui/EntryNode.h \
    simparm/cmdline_ui/Node.h simparm/cmdline_ui/Node.cpp \
    simparm/cmdline_ui/RootNode.h simparm/cmdline_ui/RootNode.cpp \
    simparm/cmdline_ui/OptionTable.h simparm/cmdline_ui/OptionTable.cpp \
    simparm/cmdline_ui/ProgressNode.h simparm/cmdline_ui/ProgressNode.cpp \
    simparm/dummy_ui/fwd.h simparm/dummy_ui/fwd.cpp

rapidstorm_SOURCES += \
    ui/serialization/Node.h ui/serialization/Node.cpp \
    ui/serialization/serialize.h ui/serialization/serialize.cpp \
    ui/serialization/deserialize.h ui/serialization/deserialize.cpp

if HAVE_LIBREADSIF
    SIFSUPPORT = \
	andor-sif/AndorSIF.h andor-sif/AndorSIF.cpp \
	andor-sif/AndorSIF_OpenFile.h andor-sif/AndorSIF_OpenFile.cpp
else
    SIFSUPPORT =
endif

rapidstorm_SOURCES += \
    $(SIFSUPPORT) \
    andor-sif/augment_config.h andor-sif/augment_config.cpp

rapidstorm_SOURCES += \
    density_map/DensityMap.h density_map/DensityMap.hpp \
    density_map/Listener.h \
    density_map/DummyListener.h density_map/DummyListener.cpp \
    density_map/VirtualListener.h density_map/VirtualListener.cpp \
    density_map/Interpolator.h density_map/InterpolatorFactory.h \
    density_map/InterpolatorChoice.h density_map/InterpolatorChoice.hpp density_map/InterpolatorChoice.cpp \
    density_map/LinearInterpolation.h density_map/LinearInterpolation.hpp density_map/LinearInterpolation.cpp \
    density_map/NearestNeighbourInterpolation.h density_map/NearestNeighbourInterpolation.hpp density_map/NearestNeighbourInterpolation.cpp \
    density_map/GaussianSmoothing.h density_map/GaussianSmoothing.hpp density_map/GaussianSmoothing.cpp \
    density_map/Coordinates.h \
    density_map/CoordinatesFactory.h density_map/CoordinatesFactory.cpp

rapidstorm_SOURCES += \
    binning/binning.h binning/binning.hpp binning/binning_decl.h binning/binning.cpp \
    binning/inversion.h \
    binning/localization.h \
    binning/localization_impl.h \
    binning/localization_config.h \
    binning/localization_config_impl.h \
    binning/config.h binning/config.cpp \
    binning/dummy_binner.h \
    binning/always_failing_binner.h \
    binning/constant_binner.h binning/constant_binner.cpp 

rapidstorm_SOURCES += \
    third_party/detail/any_iterator_abstract_base.hpp \
    third_party/detail/any_iterator_wrapper.hpp \
    third_party/detail/any_iterator_metafunctions.hpp \
    third_party/any_iterator.hpp \
    boost/units/Eigen/Core \
    boost/units/Eigen/Array \
    boost/units/systems/camera/time.hpp \
    boost/units/systems/camera/length.hpp \
    boost/units/systems/camera/resolution.hpp \
    boost/units/systems/camera/frame_rate.hpp \
    boost/units/systems/camera/area.hpp \
    boost/units/systems/camera/intensity.hpp \
    boost/units/systems/camera/intensity_dimension.hpp \
    boost/units/systems/camera/base.hpp \
    boost/units/systems/base_units/frame.hpp \
    boost/units/systems/base_units/pixel.hpp \
    boost/units/power10.hpp

if HAVE_PROTOBUF
rapidstorm_SOURCES += \
    tsf/Output.h \
    tsf/Output.cpp \
    tsf/TSFProto.pb.cc
endif

rapidstorm_LDFLAGS = $(LINKER_RESOURCE) $(RESOURCE_OBJECT) -fwhole-program 
rapidstorm_DEPENDENCIES = $(RESOURCE_OBJECT) 
rapidstorm_CPPFLAGS = -I$(srcdir)/third_party
rapidstorm_CPPFLAGS += $(GraphicsMagick_CFLAGS) $(WX_CPPFLAGS) 
rapidstorm_CXXFLAGS = $(boost_thread_CFLAGS) $(gsl_CFLAGS) $(libb64_CFLAGS)
# TODO: This flag is only needed for old Eigen3 versions up to the fix of Eigen bug 477
rapidstorm_CXXFLAGS += -Wno-enum-compare -std=c++0x -Wno-deprecated-declarations
# Workaround for https://github.com/google/protobuf/issues/1797
rapidstorm_CXXFLAGS += -Wno-misleading-indentation
rapidstorm_LDADD = $(GraphicsMagick_LIBS) 
rapidstorm_LDADD += $(boost_filesystem_LIB) $(boost_system_LIB) $(boost_thread_LIB) $(libb64_LIBS) 
rapidstorm_LDADD += $(tinyxml_LIBS) $(gsl_LIBS)
rapidstorm_LDADD += $(boost_unit_test_LIB) $(libreadsif_LIBS)
rapidstorm_LDADD += $(WX_LIBS) 
rapidstorm_LDADD += $(protobuf_LIBS)

%.pb.cc %.pb.h : %.proto
	$(PROTOC) --proto_path=$(srcdir) --cpp_out=$(builddir) $<
