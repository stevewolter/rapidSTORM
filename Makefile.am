ACLOCAL_AMFLAGS = -I m4
AUTOMAKE_OPTIONS = foreign subdir-objects
INSTALL_PROGRAM = ${INSTALL} -m 711

bin_PROGRAMS = dstorm-@DSTORM_LIB_VERSION@
C_TESTS = alignment_test \
	check_uncorrelated check_derivatives check_3d_plugin
check_PROGRAMS = $(C_TESTS)
EXTRA_PROGRAMS = testDisplay 
noinst_LTLIBRARIES = libfitter.la libAndorCamera.la \
    libinputs.la libnoopengine.la libengine.la liboutputs.la \
    libspotFinders.la libviewer.la libdStorm_display.la \
    libgauss_fitter.la libengine_stm.la

install-exec-local :
	$(RM) $(DESTDIR)$(bindir)/dstorm && ln -s dstorm-@DSTORM_LIB_VERSION@ $(DESTDIR)$(bindir)/dstorm

plugin_libdir = $(DSTORM_PLUGIN_DIR)
plugin_lib_LTLIBRARIES = lib3d_fitter.la

TESTS = $(C_TESTS)

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
    3d_fitter/Exponential3D_Accessor.h 3d_fitter/Unspecialized.cpp
lib3d_fitter_la_LDFLAGS = -no-undefined -version-info 1:0:0 -module
lib3d_fitter_la_LIBADD = -lsimparm

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

libinputs_la_SOURCES = \
	inputs/AndorSIF.cpp inputs/TIFF.cpp \
	inputs/AndorSIF.h inputs/TIFF.h inputs/inputs.h inputs/inputs.cpp \
	inputs/TIFF_OpenFile.cpp \
	inputs/AndorSIF_OpenFile.h inputs/AndorSIF_OpenFile.cpp \
	inputs/BackgroundDeviationEstimator_decl.h \
	inputs/BackgroundDeviationEstimator.cpp \
	inputs/BackgroundDeviationEstimator.h \
	inputs/Splitter_decl.h inputs/Splitter.h inputs/Splitter.cpp \
	inputs/YMirror_decl.h inputs/YMirror.h inputs/YMirror.cpp

libnoopengine_la_SOURCES = \
	noop_engine/ChainLink.cpp noop_engine/ChainLink_decl.h \
	noop_engine/ChainLink.h noop_engine/Engine.cpp noop_engine/Engine.h
libengine_la_SOURCES = \
    engine/Car_decl.h engine/Car.cpp engine/Car.h engine/Engine.cpp \
    engine/EngineDebug.h engine/Engine.h \
    engine/SigmaFitter.h engine/ImageFitter.h \
    engine/SigmaGuesser.cpp engine/SigmaGuesser_FreeFormFitter.cpp \
    engine/SigmaGuesser.h \
    engine/studentPinv.c engine/studentPinv.h \
    engine/SpotFinder.cpp engine/debug.h  \
    engine/ChainLink_decl.h engine/ChainLink.h engine/ChainLink.cpp

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
    outputs/PrecisionEstimator.h outputs/PrecisionEstimator.cpp \
    outputs/BasicTransmissions.h \
    outputs/MemoryCache.cpp outputs/MemoryCache.h outputs/MemoryCache_decl.h
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

libAndorCamera_la_SOURCES = \
    AndorCamera/CameraConnection.h AndorCamera/CameraConnection.cpp \
    AndorCamera/InputChainLink_decl.h AndorCamera/InputChainLink.h AndorCamera/InputChainLink.cpp \
    AndorCamera/LiveView.h AndorCamera/LiveView.cpp \
    AndorCamera/AndorDirect.h AndorCamera/AndorDirect.cpp AndorCamera/AndorDirect_iterator.cpp \
    AndorCamera/ViewportSelector.h AndorCamera/ViewportSelector.cpp

testDisplay_SOURCES = wxDisplay/test.cpp
testDisplay_LDADD = libdStorm_display.la
check_uncorrelated_SOURCES = gauss_fitter/check_uncorrelated.cpp
check_derivatives_SOURCES = 3d_fitter/check_derivatives.cpp
check_3d_plugin_SOURCES = 3d_fitter/check_plugin.cpp
check_3d_plugin_LDADD = lib3d_fitter.la

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
    libengine.la libnoopengine.la libgauss_fitter.la  \
    libfitter.la libinputs.la \
    libAndorCamera.la \
    libdStorm_display.la \
    -lltdl 

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

MAINPROGRAM = $(bindir)/dstorm-@DSTORM_LIB_VERSION@
CONFIGFILEBASE = dstorm-config-@DSTORM_LIB_VERSION@.txt
CONFIGFILE = $(pkgdatadir)/dstorm-config-@DSTORM_LIB_VERSION@.txt

CLEANFILES = rapidSTORM.jar \
	3d_fitter/config.h 3d_fitter/config.status \
	3d_fitter/libtool 3d_fitter/config.log \
	gauss_fitter/config.h gauss_fitter/config.status \
	gauss_fitter/libtool gauss_fitter/config.log \
	rapidstorm-@DSTORM_LIB_VERSION@.desktop rapidstorm-@DSTORM_LIB_VERSION@
	$(CONFIGFILE)

javabuilddir = classes

clean-local :
	-rm -fr $(javabuilddir)

vpath %.jar $(DESTDIR)$(datarootdir)/java

rapidSTORM.jar : $(JAVASOURCES) manifest $(TWIDDLER_JAR) $(JARCONTENT)
	$(MKDIR_P) $(javabuilddir)
	$(JAVAC) -classpath $(CLASSPATH):$(TWIDDLER_JAR): -d $(javabuilddir) $(filter %.java,$^)
	cp $(TWIDDLER_JAR) $@
	$(JAR) -uMf $@ -C $(srcdir) $(JARCONTENT) -C $(javabuilddir) .
	$(JAR) -umf manifest $@

rapidstorm-@DSTORM_LIB_VERSION@.desktop : $(srcdir)/desktop-icon.in
	sed -e 's;%MAINPROGRAM%;$(MAINPROGRAM);g' -e 's;%PACKAGE_VERSION%;$(PACKAGE_VERSION);g' -e 's;%CONFIGFILE%;$(CONFIGFILE);' $< > $@

rapidstorm-@DSTORM_LIB_VERSION@ : $(srcdir)/menu.in
	sed -e 's;%MAINPROGRAM%;$(MAINPROGRAM);g' -e 's;%PACKAGE_VERSION%;$(PACKAGE_VERSION);g' -e 's;%CONFIGFILE%;$(CONFIGFILE);' $< > $@

$(CONFIGFILEBASE) : dstorm-config.txt
	cat $< > $@

menudir=$(sysconfdir)/menu
menu_DATA=rapidstorm-@DSTORM_LIB_VERSION@
applicationsdir=$(datadir)/applications
applications_DATA=rapidstorm-@DSTORM_LIB_VERSION@.desktop

pkgdata_DATA = $(CONFIGFILEBASE)
