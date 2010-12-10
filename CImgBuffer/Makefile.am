lib_LTLIBRARIES = libCImgBuffer.la
libCImgBuffer_la_SOURCES = ImageVector.cpp ImageSources.cpp \
    AndorSIF.cpp Config.cpp AndorDirect.h AndorSIF.h ViewportSelector.h \
    foreach.h Slot.cpp Pixel.h InputMethod.cpp
library_includedir = $(includedir)/CImgBuffer
library_include_HEADERS = CImgBuffer/Image.h CImgBuffer/Source.h CImgBuffer/Buffer.h CImgBuffer/SerializedSource.h CImgBuffer/Config.h CImgBuffer/Slot.h CImgBuffer/Drain.h CImgBuffer/InputMethod.h
libCImgBuffer_la_LDFLAGS = -no-undefined -version-info 1:0:0
libCImgBuffer_la_LIBADD = $(LIBOBJS)
libCImgBuffer_la_CPPFLAGS = -D_POSIX_SOURCE -Wall -Wextra \
    -include $(top_builddir)/config.h -include $(top_builddir)/debug.h \
    -I$(srcdir)/CImgBuffer

libCImgBuffer_la_LDFLAGS += $(LIBDATACPP_LDFLAGS)
libCImgBuffer_la_CPPFLAGS += $(LIBDATACPP_CPPFLAGS)
libCImgBuffer_la_LIBADD += $(LIBDATACPP_LIBS)

libCImgBuffer_la_LDFLAGS += $(LIBREADSIF_LDFLAGS)
libCImgBuffer_la_CPPFLAGS += $(LIBREADSIF_CPPFLAGS)
libCImgBuffer_la_LIBADD += $(LIBREADSIF_LIBS)

libCImgBuffer_la_LDFLAGS += $(LIBSIMPARM_LDFLAGS)
libCImgBuffer_la_CPPFLAGS += $(LIBSIMPARM_CPPFLAGS)
libCImgBuffer_la_LIBADD += $(LIBSIMPARM_LIBS)

libCImgBuffer_la_LDFLAGS += $(LIBCOMMONCPP_LDFLAGS)
libCImgBuffer_la_CPPFLAGS += $(LIBCOMMONCPP_CPPFLAGS)
libCImgBuffer_la_LIBADD += $(LIBCOMMONCPP_LIBS)

libCImgBuffer_la_LIBADD += $(LIBCIMG_LIBS)

AD_FILES = AndorDirect.cpp AndorDirect.h \
             ViewportSelector.h ViewportSelector.cpp
EXTRA_DIST = 

if BUILD_ADSTORM
    libCImgBuffer_la_SOURCES += $(AD_FILES)
else
    EXTRA_DIST += $(AD_FILES)
endif
libCImgBuffer_la_LDFLAGS += $(LIBANDORCAMERA_LDFLAGS)
libCImgBuffer_la_CPPFLAGS += $(LIBANDORCAMERA_CPPFLAGS)
libCImgBuffer_la_LIBADD += $(LIBANDORCAMERA_LIBS) 
