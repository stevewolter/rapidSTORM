ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign

SUBDIRS = fit++ AndorCamera inputs engine spotFinders outputs wxDisplay viewer

bin_PROGRAMS = dstorm 

EXTRA_DATA = rapidSTORM.jar
SOURCES = DStorm.java
EXTRA_DIST = $(SOURCES) manifest logo.png

dist_pkgdata_DATA = dstorm-config.txt logo.png manifest

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

COMMON_LIBS = outputs/liboutputs.la viewer/libviewer.la \
    spotFinders/libspotFinders.la \
    engine/libengine.la inputs/libinputs.la \
    AndorCamera/libAndorCamera.la \
    wxDisplay/libdStorm_display.la \
    ../dStorm/libdStorm.la 
dstorm_SOURCES = \
    dstorm.cpp \
    CommandLine.h CommandLine.cpp \
    ModuleLoader.h ModuleLoader.cpp \
    LibraryHandle.h LibraryHandle.cpp \
    GarageConfig.h GarageConfig.cpp \
    InputStream.h InputStream.cpp \
    debug.h error_handler.h error_handler.cpp \
		 $(RESOURCE_FILE)
dstorm_LDADD = $(COMMON_LIBS) -lltdl 
    
javadir=$(datadir)/java
nobase_java_DATA = de/uni_bielefeld/physik/rapid2STORM/DStorm.class

de/uni_bielefeld/physik/rapid2STORM/DStorm.class : $(SOURCES)
	$(JAVAC) -d $(builddir) -classpath $(CLASSPATH):$(TWIDDLER_JAR) $^
