bin_PROGRAMS = dstorm watchStorm
lib_LTLIBRARIES = libdStorm.la

nobase_include_HEADERS = \
    dStorm/BasicOutputs.h \
    dStorm/Candidate.h \
    dStorm/CandidateTree.h \
    dStorm/CarConfig.h \
    dStorm/Config.h \
    dStorm/FileOutputBuilder.h \
    dStorm/FilterBuilder.h \
    dStorm/FilterSource.h \
    dStorm/Image.h \
    dStorm/Input.h \
    dStorm/Localization.h \
    dStorm/ModuleInterface.h \
    dStorm/OutputBuilder.h \
    dStorm/OutputFactory.h \
    dStorm/Output.h \
    dStorm/OutputSource.h \
    dStorm/ResultRepeater.h \
    dStorm/Spot.h

EXTRA_DATA = rapidSTORM.jar
SOURCES = DStorm.java
EXTRA_DIST = $(SOURCES) manifest

LDADD_BASIC =
LDFLAGS_BASIC = 
CPPFLAGS_BASIC = -D_POSIX_SOURCE -Wall -Wextra \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h
LIBRARY_OBJECTS=

libdStorm_la_SOURCES = \
    BasicOutputs.cpp \
    CandidateTree.cpp \
    CarConfig.cpp \
    Config.cpp \
    dStorm \
    FilterSource.cpp \
    foreach.h \
    Input.cpp \
    Localization.cpp \
    OutputSource.cpp
libdStorm_la_CPPFLAGS = -D_GNU_SOURCE \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    --param inline-unit-growth=1000
libdStorm_la_LDFLAGS = -no-undefined -version-info 1:0:0

dstorm_SOURCES = dstorm.cpp Garage.cpp GarageConfig.h Garage.h logo.cpp
watchStorm_SOURCES = watchStorm.cpp Garage.cpp logo.cpp

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
