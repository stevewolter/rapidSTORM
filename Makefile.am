ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign

if BUILD_DEBUG_PLUGIN
    DEBUGPLUGIN = debug-plugin
else
    DEBUGPLUGIN =
endif

SUBDIRS = fit++ AndorCamera inputs engine gauss_fitter spotFinders outputs wxDisplay viewer $(DEBUGPLUGIN)

bin_PROGRAMS = dstorm 

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

COMMON_LIBS = outputs/liboutputs.la viewer/libviewer.la \
    spotFinders/libspotFinders.la \
    engine/libengine.la gauss_fitter/libgauss_fitter.la inputs/libinputs.la \
    AndorCamera/libAndorCamera.la \
    wxDisplay/libdStorm_display.la \
    ../dStorm/libdStorm.la 
dstorm_SOURCES = \
    dstorm.cpp \
    CommandLine.h CommandLine.cpp \
    ModuleLoader.h ModuleLoader.cpp \
    LibraryHandle.h LibraryHandle.cpp \
    JobStarter.h JobStarter.cpp \
    InputStream.h InputStream.cpp \
    debug.h $(RESOURCE_FILE) \
    local_cleanup.h local_cleanup.cpp
dstorm_LDADD = $(COMMON_LIBS) -lltdl 
    
javadir=$(datadir)/java
java_DATA = rapidSTORM.jar
JAVASOURCES = DStorm.java
JARCONTENT = logo.png
EXTRA_DIST = $(JAVASOURCES) $(JARCONTENT) manifest

javabuilddir = classes

rapidSTORM.jar : $(JAVASOURCES) manifest $(TWIDDLER_JAR) $(JARCONTENT)
	$(MKDIR_P) $(javabuilddir)
	$(JAVAC) -classpath $(CLASSPATH):$(TWIDDLER_JAR): -d $(javabuilddir) $(filter %.java,$^)
	cp $(TWIDDLER_JAR) $@
	$(JAR) -uMf $@ -C $(srcdir) $(JARCONTENT) -C $(javabuilddir) .
	$(JAR) -umf manifest $@
