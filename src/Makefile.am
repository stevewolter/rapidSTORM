ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign

SUBDIRS = fit++ AndorCamera inputs engine spotFinders outputs wxDisplay

bin_PROGRAMS = dstorm watchStorm

EXTRA_DATA = rapidSTORM.jar
SOURCES = DStorm.java
EXTRA_DIST = $(SOURCES) manifest logo.png

dist_pkgdata_DATA = dstorm-config.txt logo.png manifest

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    '-DDSTORM_PLUGIN_DIR="${DSTORM_PLUGIN_DIR}"'

COMMON_LIBS = outputs/liboutputs.la spotFinders/libspotFinders.la \
    engine/libengine.la inputs/libinputs.la \
    wxDisplay/libdStorm_display.la \
    AndorCamera/libAndorCamera.la ../dStorm/libdStorm.la 
dstorm_SOURCES = dstorm.cpp Garage.cpp GarageConfig.h Garage.h \
                 MasterConfig.h MasterConfig.cpp GarageConfig.cpp \
		 debug.h
dstorm_LDADD = $(COMMON_LIBS) -lltdl -lboost_thread-mt
    
watchStorm_SOURCES = watchStorm.cpp Garage.cpp \
                     MasterConfig.h MasterConfig.cpp GarageConfig.cpp
watchStorm_LDADD = $(COMMON_LIBS) -lltdl -lboost_thread-mt

javadir=$(datadir)/java
java_DATA = DStorm.class
TWIDDLER = $(javadir)/twiddler.jar

DStorm.class : $(SOURCES)
	$(JAVAC) -d $(builddir) -classpath $(CLASSPATH):$(TWIDDLER) $^
