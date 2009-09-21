lib_LTLIBRARIES = libCImgBuffer.la
libCImgBuffer_la_SOURCES = \
    AndorSIF.cpp Config.cpp ViewportSelector.h \
    foreach.h \
    CImgList.h CImgList.cpp ImageSource.cpp \
    Source.cpp TIFF.cpp
library_includedir = $(includedir)/CImgBuffer
library_include_HEADERS = CImgBuffer/Source.h CImgBuffer/Buffer.h CImgBuffer/Config.h CImgBuffer/Slot.h CImgBuffer/Drain.h CImgBuffer/InputMethod.h \
	CImgBuffer/Traits.h CImgBuffer/ImageTraits.h \
	CImgBuffer/Buffer_impl.h CImgBuffer/Slot_impl.h \
	CImgBuffer/SerialImageSource.h CImgBuffer/AndorSIF.h \
	CImgBuffer/TIFF.h CImgBuffer/AndorDirect.h
libCImgBuffer_la_LDFLAGS = -no-undefined -version-info 1:0:0
libCImgBuffer_la_LIBADD = $(LIBOBJS)
libCImgBuffer_la_CPPFLAGS = -D_POSIX_SOURCE -Wall -Wextra \
    -include $(top_builddir)/config.h -include $(top_builddir)/debug.h \
    -I$(srcdir)/CImgBuffer
libCImgBuffer_la_DEPENDENCIES =

libCImgBuffer_la_LDFLAGS += $(LIBDATACPP_LDFLAGS)
libCImgBuffer_la_CPPFLAGS += $(LIBDATACPP_CPPFLAGS)
libCImgBuffer_la_LIBADD += $(LIBDATACPP_LIBS) $(LIBDATACPP_OBJECTS)
libCImgBuffer_la_DEPENDENCIES += $(LIBDATACPP_OBJECTS)

libCImgBuffer_la_LDFLAGS += $(LIBREADSIF_LDFLAGS)
libCImgBuffer_la_CPPFLAGS += $(LIBREADSIF_CPPFLAGS)
libCImgBuffer_la_LIBADD += $(LIBREADSIF_LIBS) $(LIBREADSIF_OBJECTS)
libCImgBuffer_la_DEPENDENCIES += $(LIBREADSIF_OBJECTS)

libCImgBuffer_la_LDFLAGS += $(LIBSIMPARM_LDFLAGS)
libCImgBuffer_la_CPPFLAGS += $(LIBSIMPARM_CPPFLAGS)
libCImgBuffer_la_LIBADD += $(LIBSIMPARM_LIBS) $(LIBSIMPARM_OBJECTS)
libCImgBuffer_la_DEPENDENCIES += $(LIBSIMPARM_OBJECTS)

libCImgBuffer_la_LDFLAGS += $(LIBCOMMONCPP_LDFLAGS)
libCImgBuffer_la_CPPFLAGS += $(LIBCOMMONCPP_CPPFLAGS)
libCImgBuffer_la_LIBADD += $(LIBCOMMONCPP_LIBS) $(LIBCOMMONCPP_OBJECTS)
libCImgBuffer_la_DEPENDENCIES += $(LIBCOMMONCPP_OBJECTS)

libCImgBuffer_la_LDFLAGS += $(LIBCIMG_LDFLAGS)
libCImgBuffer_la_CPPFLAGS += $(LIBCIMG_CPPFLAGS)
libCImgBuffer_la_LIBADD += $(LIBCIMG_LIBS) $(LIBCIMG_OBJECTS)
libCImgBuffer_la_DEPENDENCIES += $(LIBCIMG_OBJECTS)

AD_FILES = AndorDirect.cpp \
             ViewportSelector.h ViewportSelector.cpp
EXTRA_DIST = 

if BUILD_ADSTORM
    libCImgBuffer_la_SOURCES += $(AD_FILES)
    libCImgBuffer_la_LDFLAGS += $(LIBANDORCAMERA_LDFLAGS)
    libCImgBuffer_la_CPPFLAGS += $(LIBANDORCAMERA_CPPFLAGS)
    libCImgBuffer_la_LIBADD += $(LIBANDORCAMERA_LIBS) \
                               $(LIBANDORCAMERA_OBJECTS)
    libCImgBuffer_la_DEPENDENCIES += $(LIBANDORCAMERA_OBJECTS)
else
    EXTRA_DIST += $(AD_FILES)
endif

if BUILD_STATIC_PROGRAMS
    libCImgBuffer_la_LDFLAGS += -static
endif

check_PROGRAMS = $(C_TESTS)
check_AndorCamera_with_DummyCamera_SOURCES = \
    check_AndorCamera_with_DummyCamera.cpp
check_AndorCamera_with_DummyCamera_LDFLAGS = \
    $(libCImgBuffer_la_LDFLAGS) $(ANDOR_SDK_LDFLAGS)
check_AndorCamera_with_DummyCamera_CPPFLAGS = \
    $(libCImgBuffer_la_CPPFLAGS) $(ANDOR_SDK_CPPFLAGS)
check_AndorCamera_with_DummyCamera_LDADD = \
    $(libCImgBuffer_la_LIBADD) $(ANDOR_SDK_OBJECTS) \
    $(ANDOR_SDK_LIBS) libCImgBuffer.la
check_AndorCamera_with_DummyCamera_DEPENDECIES = \
    $(libCImgBuffer_la_DEPENDECIES) $(ANDOR_SDK_OBJECTS) libCImgBuffer.la

C_TESTS = check_AndorCamera_with_DummyCamera
TESTS = $(C_TESTS)
