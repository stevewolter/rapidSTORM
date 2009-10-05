SUBDIRS = cc++ fit++ data-c++ AndorCamera CImgBuffer dStorm engine outputs

lib_LTLIBRARIES = libdStorm.la
bin_PROGRAMS = dstorm watchStorm

EXTRA_DATA = rapidSTORM.jar
SOURCES = DStorm.java
EXTRA_DIST = $(SOURCES) manifest

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h

libdStorm_la_SOURCES = 
nodist_EXTRA_libdStorm_la_SOURCES = dummy.cpp
libdStorm_la_LIBADD = outputs/libfreeoutputs.la dStorm/libdStorm.la \
    CImgBuffer/libCImgBuffer.la AndorCamera/libAndorCamera.la \
    cc++/libccgnu2_fixed.la -lltdl 
libdStorm_la_LDFLAGS = -no-undefined -version-info 1:0:0 

COMMON_LIBS = outputs/libclosedoutputs.la engine/libengine.la libdStorm.la
dstorm_SOURCES = dstorm.cpp Garage.cpp GarageConfig.h Garage.h logo.cpp 
dstorm_LDADD = $(COMMON_LIBS)
    
watchStorm_SOURCES = watchStorm.cpp Garage.cpp logo.cpp
watchStorm_LDADD = $(COMMON_LIBS)

javadir=$(srcdir)
SIMPARM_DIR=../simparm/twiddler
TWIDDLER=../simparm/twiddler/twiddler.jar
DLLDIR=/usr/local/i586-mingw32msvc/dll
DLLs=pthreadGCE2.dll ATMCD32D.DLL mingwm10.dll

dist-jar : rapidSTORM.jar

DStorm.class : $(SOURCES)
	$(JAVAC) -d $(builddir) -classpath $(CLASSPATH):$(TWIDDLER) $^

rapidSTORM.jar : $(TWIDDLER) DStorm.class dstorm$(EXEEXT) manifest rapidSTORM.chm
	$(JAR) -cmf manifest $@ \
	    -C . DStorm.class dstorm$(EXEEXT) rapidSTORM.chm \
	    -C $(SIMPARM_DIR)/classes . \
	    $(addprefix -C $(DLLDIR) ,$(DLLs)) \
	    -C $(srcdir) dstorm-config.txt
