ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign

SUBDIRS = fit++ AndorCamera inputs engine spotFinders outputs

bin_PROGRAMS = dstorm watchStorm

EXTRA_DATA = rapidSTORM.jar
SOURCES = DStorm.java
EXTRA_DIST = $(SOURCES) manifest logo.png

dist_pkgdata_DATA = dstorm-config.txt logo.png manifest

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/../debug.h \
    '-DDSTORM_PLUGIN_DIR="${DSTORM_PLUGIN_DIR}"'

COMMON_LIBS = outputs/liboutputs.la spotFinders/libspotFinders.la \
    engine/libengine.la inputs/libinputs.la \
    AndorCamera/libAndorCamera.la ../dStorm/libdStorm.la 
dstorm_SOURCES = dstorm.cpp Garage.cpp GarageConfig.h Garage.h 
dstorm_LDADD = $(COMMON_LIBS) -lltdl 
    
watchStorm_SOURCES = watchStorm.cpp Garage.cpp 
watchStorm_LDADD = $(COMMON_LIBS) -lltdl

javadir=$(datadir)/java
java_DATA = DStorm.class
TWIDDLER = $(javadir)/twiddler.jar

DStorm.class : $(SOURCES)
	$(JAVAC) -d $(builddir) -classpath $(CLASSPATH):$(TWIDDLER) $^
