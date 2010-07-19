ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign subdir-objects

if BUILD_DEBUG_PLUGIN
    DEBUGPLUGIN = libtestplugin.la
else
    DEBUGPLUGIN =
endif

bin_PROGRAMS = dstorm 
C_TESTS = alignment_test \
	check_uncorrelated check_derivatives check_plugin
check_PROGRAMS = $(C_TESTS)
EXTRA_PROGRAMS = testDisplay 
noinst_LTLIBRARIES = libfitter.la libAndorCamera.la \
    libinputs.la libengine.la liboutputs.la \
    libspotFinders.la libviewer.la libdStorm_display.la \
    libgauss_fitter.la

plugin_libdir = $(DSTORM_PLUGIN_DIR)
plugin_lib_LTLIBRARIES = $(DEBUGPLUGIN) lib3d_fitter.la

TESTS = $(C_TESTS)

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    '-DDSTORM_PLUGIN_DIR="${DSTORM_PLUGIN_DIR}"'

if HAVE_WINDRES
    RESOURCE_FILE = windows_resources.rc
else
    RESOURCE_FILE =
endif

.rc.o : 
	$(WINDRES) -I . '$<' '$@'
windows_resources.o : config.h

lib3d_fitter_la_SOURCES = \
    3d_fitter/Config_decl.h 3d_fitter/Config.h 3d_fitter/Config.cpp \
    3d_fitter/Factory.h 3d_fitter/Factory.cpp \
    3d_fitter/Exponential3D.hh 3d_fitter/Exponential3D_ParameterHelper.hh \
    3d_fitter/Exponential3D_impl.hh 3d_fitter/Exponential3D_Derivatives.h \
    3d_fitter/Fitter.h 3d_fitter/Fitter.cpp \
    3d_fitter/Specialized.cpp 3d_fitter/Specialized_naive.cpp \
    3d_fitter/plugin.cpp 3d_fitter/ConstantTypes.h \
    3d_fitter/Exponential3D_Accessor.h
lib3d_fitter_la_LDFLAGS = -no-undefined -version-info 1:0:0 -module
lib3d_fitter_la_LIBADD = ../dStorm/libdStorm.la -lsimparm

libgauss_fitter_la_SOURCES = \
    gauss_fitter/Config_decl.h gauss_fitter/Config.h gauss_fitter/Config.cpp \
    gauss_fitter/Factory.h gauss_fitter/Factory.cpp \
    gauss_fitter/no_analysis/main.cpp gauss_fitter/no_analysis/main.h \
    gauss_fitter/no_analysis/fitters_uncorrelated.cpp \
    gauss_fitter/no_analysis/fitters_correlated.cpp \
    gauss_fitter/no_analysis/impl.h \
    gauss_fitter/residue_analysis/main.cpp \
    gauss_fitter/residue_analysis/main.h gauss_fitter/residue_analysis/impl.h \
    gauss_fitter/residue_analysis/fitters_uncorrelated_0.cpp \
    gauss_fitter/residue_analysis/fitters_uncorrelated_1.cpp \
    gauss_fitter/residue_analysis/fitters_correlated.cpp \
    fit++/Exponential2D_Correlated_Derivatives.hh \
    fit++/Exponential2D.hh \
    fit++/Exponential2D_impl.hh \
    fit++/Exponential2D_ParameterHelper.hh \
    fit++/Exponential2D_Uncorrelated_Derivatives.hh
libgauss_fitter_la_CPPFLAGS = -I$(srcdir)/gauss_fitter

libtestplugin_la_LDFLAGS = -no-undefined -version-info 1:0:0 -module
libtestplugin_la_SOURCES = debug-plugin/plugin.cpp debug-plugin/SegmentationFault.h \
    debug-plugin/MuchMemoryAllocator.h debug-plugin/Exception.h debug-plugin/Delayer.h \
    debug-plugin/BasenamePrinter.h debug-plugin/BasenamePrinter.cpp \
    debug-plugin/Manager.h debug-plugin/Manager.cpp \
    debug-plugin/md5.h debug-plugin/md5.c \
    debug-plugin/DummyFileInput.h debug-plugin/DummyFileInput.cpp \
    debug-plugin/DummyFitter.h debug-plugin/DummyFitter.cpp \
    debug-plugin/Verbose.h 
libtestplugin_la_LIBADD = $(top_builddir)/../dStorm/libdStorm.la -lsimparm -lpthread

libinputs_la_SOURCES = \
	inputs/AndorSIF.cpp inputs/TIFF.cpp \
	inputs/AndorSIF.h inputs/TIFF.h inputs/inputs.h inputs/inputs.cpp

libengine_la_SOURCES = \
    engine/Car_decl.h engine/Car.cpp engine/Car.h engine/Engine.cpp \
    engine/EngineDebug.h engine/Engine.h \
    engine/LocalizationBuncher.cpp engine/LocalizationBuncher.h \
    engine/SigmaFitter.h engine/ImageFitter.h \
    engine/SigmaGuesser.cpp engine/SigmaGuesser_FreeFormFitter.cpp \
    engine/SigmaGuesser.h \
    engine/studentPinv.c engine/studentPinv.h \
    engine/SpotFinder.cpp engine/debug.h  \
    engine/ThresholdGuesser.h engine/ThresholdGuesser.cpp

liboutputs_la_SOURCES = \
    outputs/AverageImage.cpp outputs/AverageImage.h outputs/BasicTransmissions.cpp \
    outputs/LocalizationCounter.cpp outputs/LocalizationCounter.h \
    outputs/LocalizationFile.cpp outputs/LocalizationFile.h \
    outputs/LocalizationFilter.cpp outputs/LocalizationFilter.h \
    outputs/ProgressMeter.cpp \
    outputs/ProgressMeter.h outputs/RawImageFile.cpp outputs/RawImageFile.h outputs/Slicer.cpp \
    outputs/Slicer.h \
    outputs/PrecisionEstimator.h outputs/PrecisionEstimator.cpp \
    outputs/BasicTransmissions.h
liboutputs_la_LIBADD = -lberghen_xmlParser

libspotFinders_la_SOURCES = \
    spotFinders/averageSmooth.cpp \
    spotFinders/averageSmooth.h \
    spotFinders/ErosionSmoother.h \
    spotFinders/GaussSmoothing.cpp \
    spotFinders/GaussSmoothing.h \
    spotFinders/Median.cpp \
    spotFinders/MedianSmoother.h \
    spotFinders/Spalttiefpass.h \
    spotFinders/spotFinders.cpp spotFinders/spotFinders.h

libviewer_la_SOURCES = \
    viewer/Colorizer_decl.h viewer/ColourDisplay.cpp viewer/ColourDisplay.h viewer/ColourDisplay_impl.h \
    viewer/Display.h viewer/Display_inline.h viewer/Display_impl.h viewer/Display.cpp \
    viewer/ImageDiscretizer.h viewer/ImageDiscretizer_inline.h \
	viewer/ImageDiscretizer_impl.h viewer/ImageDiscretizer.cpp \
    viewer/Config_decl.h viewer/Config.h viewer/Config.cpp \
    viewer/Backend_decl.h viewer/Backend.h \
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
    viewer/plugin.h

libfitter_la_SOURCES = \
    fit++/BitfieldConstructor.hh fit++/BlockReturner.hh \
    fit++/Exponential_Common_decl.h fit++/Exponential_Common.h \
    fit++/Exponential_Common_ParameterHelper.hh \
    fit++/Exponential_Uncorrelated_Derivatives.h \
    fit++/FitFunction.hh fit++/FitFunction_impl.hh \
    fit++/Fitter.hh fit++/Helpers.hh \
    fit++/LatticeFitFunction.hh fit++/LeastSquaresLatticeFitter.hh \
    fit++/ParameterAllocator.hh fit++/Position.hh \
    fit++/ResidueMatrix.hh \
    fitter/Sized.h fitter/FixedSized.h \
    fitter/FixedSized_impl.h \
    fitter/SizeSpecializing_decl.h fitter/SizeSpecializing.h \
    fitter/SizeSpecializing_impl.h \
    fitter/SizeSpecializing_filler.h \
    fitter/MarquardtConfig.h \
    fitter/residue_analysis/analysis.h \
    fitter/residue_analysis/Config_decl.h \
    fitter/residue_analysis/Config.h \
    fitter/residue_analysis/Config_impl.h \
    fitter/residue_analysis/fitter.h \
    fitter/residue_analysis/impl.h \
    fitter/residue_analysis/main.h \
    fitter/FixedSized_decl.h \
    fitter/MarquardtInfo.h fitter/MarquardtInfo_impl.h \
    fitter/MarquardtConfig_impl.h

libdStorm_display_la_SOURCES = \
    wxDisplay/wxManager.h wxDisplay/wxManager.cpp wxDisplay/App.h wxDisplay/App.cpp \
    wxDisplay/Window.h wxDisplay/Window.cpp wxDisplay/Canvas.h wxDisplay/Canvas.cpp \
    wxDisplay/ZoomSlider.h wxDisplay/ZoomSlider.cpp wxDisplay/Key.h wxDisplay/Key.cpp \
    wxDisplay/helpers.h wxDisplay/ScaleBar.h wxDisplay/ScaleBar.cpp wxDisplay/helpers.cpp \
    wxDisplay/wxManager_store_image.cpp wxDisplay/SizeConvert.h

if BUILD_ANDORCAM
    ACS = \
	AndorCamera/SDK.h AndorCamera/SDK.cpp AndorCamera/Error.cpp AndorCamera/foreach.h \
	AndorCamera/Acquisition.cpp AndorCamera/StateMachine.cpp \
	AndorCamera/StateMachine.h AndorCamera/StateMachine_impl.h \
	AndorCamera/TemperatureMonitor.h AndorCamera/TemperatureMonitor.cpp AndorCamera/Error.h \
	AndorCamera/System.cpp AndorCamera/Camera.cpp \
	AndorCamera/AcquisitionSwitch.h AndorCamera/AcquisitionSwitch.cpp AndorCamera/Initialization.cpp \
	AndorCamera/Temperature.cpp AndorCamera/Readout.cpp AndorCamera/AcquisitionMode.cpp \
	AndorCamera/ShutterControl.cpp AndorCamera/Triggering.cpp AndorCamera/ShiftSpeedControl.cpp \
	AndorCamera/Gain.cpp AndorCamera/AndorDirect.cpp AndorCamera/AndorDirect_iterator.cpp AndorCamera/AndorDirect.h \
	AndorCamera/ViewportSelector.h AndorCamera/ViewportSelector.cpp \
	AndorCamera/Initialization.h AndorCamera/AcquisitionMode.h \
	AndorCamera/Acquisition.h AndorCamera/Readout.h AndorCamera/Camera.h AndorCamera/Triggering.h AndorCamera/CameraReference.h \
	AndorCamera/System.h AndorCamera/Temperature.h AndorCamera/ShiftSpeedControl.h AndorCamera/Gain.h AndorCamera/ShutterControl.h \
	AndorCamera/AndorDirect_decl.h AndorCamera/LiveView_decl.h AndorCamera/LiveView.h \
	AndorCamera/LiveView.cpp AndorCamera/EmergencyHandler.h AndorCamera/EmergencyHandler.cpp
else
    ACS = 
endif
libAndorCamera_la_SOURCES = AndorCamera/Config.cpp AndorCamera/Config.h $(ACS)
testDisplay_SOURCES = wxDisplay/test.cpp
testDisplay_LDADD = ../dStorm/libdStorm.la libdStorm_display.la
check_uncorrelated_SOURCES = gauss_fitter/check_uncorrelated.cpp
check_derivatives_SOURCES = 3d_fitter/check_derivatives.cpp
check_plugin_SOURCES = 3d_fitter/check_plugin.cpp
check_plugin_LDADD = lib3d_fitter.la

COMMON_LIBS = liboutputs.la libviewer.la \
    libspotFinders.la \
    libengine.la libgauss_fitter.la  \
    libfitter.la libinputs.la \
    libAndorCamera.la \
    libdStorm_display.la \
    ../dStorm/libdStorm.la 

dstorm_SOURCES = \
    dstorm.cpp \
    CommandLine.h CommandLine.cpp \
    ModuleLoader.h ModuleLoader.cpp \
    LibraryHandle.h LibraryHandle.cpp \
    JobStarter.h JobStarter.cpp \
    InputStream.h InputStream.cpp \
    debug.h debug_print.h $(RESOURCE_FILE) \
    local_cleanup.h local_cleanup.cpp \
    TIFFOperation.h TIFFOperation.cpp
dstorm_LDADD = $(COMMON_LIBS) -lltdl 

alignment_test_LDADD = ../dStorm/libdStorm.la
alignment_test_SOURCES = fit++/alignment_test.cpp
    
javadir=$(datadir)/java
java_DATA = rapidSTORM.jar 
JAVASOURCES = DStorm.java
JARCONTENT = logo.png
EXTRA_DIST = $(JAVASOURCES) $(JARCONTENT) manifest \
 gauss_fitter/aclocal.m4 \
 gauss_fitter/config.h.in \
 gauss_fitter/configure \
 gauss_fitter/configure.ac \
 3d_fitter/aclocal.m4 \
 3d_fitter/config.h.in \
 3d_fitter/configure \
 3d_fitter/configure.ac 

CLEANFILES = rapidSTORM.jar \
	3d_fitter/config.h 3d_fitter/config.status \
	3d_fitter/libtool 3d_fitter/config.log \
	gauss_fitter/config.h gauss_fitter/config.status \
	gauss_fitter/libtool gauss_fitter/config.log

javabuilddir = classes

clean-local :
	-rm -fr $(javabuilddir)

rapidSTORM.jar : $(JAVASOURCES) manifest $(TWIDDLER_JAR) $(JARCONTENT)
	$(MKDIR_P) $(javabuilddir)
	$(JAVAC) -classpath $(CLASSPATH):$(TWIDDLER_JAR): -d $(javabuilddir) $(filter %.java,$^)
	cp $(TWIDDLER_JAR) $@
	$(JAR) -uMf $@ -C $(srcdir) $(JARCONTENT) -C $(javabuilddir) .
	$(JAR) -umf manifest $@
