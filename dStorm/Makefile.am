lib_LTLIBRARIES = libdStorm.la
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
libdStorm_la_LIBADD = $(LIBOBJS)
libdStorm_la_DEPENDENCIES = 


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

libdStorm_la_LDFLAGS += $(LIBCOMMONCPP_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBCOMMONCPP_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBCOMMONCPP_LIBS) $(LIBCOMMONCPP_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBCOMMONCPP_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBDATACPP_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBDATACPP_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBDATACPP_LIBS) $(LIBDATACPP_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBDATACPP_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBCIMGBUFFER_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBCIMGBUFFER_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBCIMGBUFFER_LIBS) $(LIBCIMGBUFFER_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBCIMGBUFFER_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBLIBFITPP_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBLIBFITPP_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBLIBFITPP_LIBS) $(LIBLIBFITPP_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBLIBFITPP_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBSIMPARM_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBSIMPARM_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBSIMPARM_LIBS) $(LIBSIMPARM_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBSIMPARM_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBCIMG_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBCIMG_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBCIMG_LIBS) $(LIBCIMG_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBCIMG_OBJECTS)

libdStorm_la_LDFLAGS += $(LIBMAGICK_LDFLAGS)
libdStorm_la_CPPFLAGS += $(LIBMAGICK_CPPFLAGS)
libdStorm_la_LIBADD += $(LIBMAGICK_LIBS) $(LIBMAGICK_OBJECTS)
libdStorm_la_DEPENDENCIES += $(LIBMAGICK_OBJECTS)

if BUILD_ADSTORM
    libdStorm_la_LDFLAGS += $(LIBANDORCAMERA_LDFLAGS)
    libdStorm_la_CPPFLAGS += $(LIBANDORCAMERA_CPPFLAGS)
    libdStorm_la_LIBADD += $(LIBANDORCAMERA_LIBS) \
                               $(LIBANDORCAMERA_OBJECTS)
    libdStorm_la_DEPENDENCIES += $(LIBANDORCAMERA_OBJECTS)
endif

