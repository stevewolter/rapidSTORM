SUBDIRS = cc++ fit++ data-c++ AndorCamera CImgBuffer dStorm engine outputs

lib_LTLIBRARIES = libdStorm.la
bin_PROGRAMS = dstorm watchStorm

EXTRA_DATA = rapidSTORM.jar
SOURCES = DStorm.java
EXTRA_DIST = $(SOURCES) manifest logo.png

dist_pkgdata_DATA = dstorm-config.txt logo.png manifest

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    '-DDSTORM_PLUGIN_DIR="${DSTORM_PLUGIN_DIR}"'

libdStorm_la_SOURCES = foreach.h
nodist_EXTRA_libdStorm_la_SOURCES = dummy.cpp
libdStorm_la_LIBADD = outputs/libfreeoutputs.la dStorm/libdStorm.la \
    CImgBuffer/libCImgBuffer.la AndorCamera/libAndorCamera.la \
    cc++/libccgnu2_fixed.la -lsimparm
libdStorm_la_LDFLAGS = -no-undefined -version-info 1:0:0 

COMMON_LIBS = outputs/libclosedoutputs.la engine/libengine.la libdStorm.la
dstorm_SOURCES = dstorm.cpp Garage.cpp GarageConfig.h Garage.h 
dstorm_LDADD = $(COMMON_LIBS) -lltdl 
    
watchStorm_SOURCES = watchStorm.cpp Garage.cpp 
watchStorm_LDADD = $(COMMON_LIBS) -lltdl

javadir=$(datadir)/java
java_DATA = DStorm.class
TWIDDLER = $(javadir)/twiddler.jar

DStorm.class : $(SOURCES)
	$(JAVAC) -d $(builddir) -classpath $(CLASSPATH):$(TWIDDLER) $^
