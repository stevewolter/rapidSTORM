noinst_LTLIBRARIES = libCImgBuffer.la
libCImgBuffer_la_SOURCES = \
    AndorSIF.cpp Config.cpp ViewportSelector.h \
    foreach.h \
    CImgList.h CImgList.cpp ImageSource.cpp \
    Source.cpp TIFF.cpp ImageTraits.cpp \
    AutoInputMethod.h
library_includedir = $(includedir)/CImgBuffer
library_include_HEADERS = Source.h Source_impl.h Buffer.h Config.h Slot.h \
	Drain.h InputMethod.h \
	Traits.h ImageTraits.h \
	Buffer_impl.h Slot_impl.h \
	SerialImageSource.h AndorSIF.h \
	TIFF.h AndorDirect.h BasenameWatcher.h
AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    -I $(top_srcdir)/src

AD_FILES = AndorDirect.cpp \
             ViewportSelector.h ViewportSelector.cpp
EXTRA_DIST = 

if HAVE_LIBATMCD32D
    libCImgBuffer_la_SOURCES += $(AD_FILES)
else
    EXTRA_DIST += $(AD_FILES)
endif

check_PROGRAMS = $(C_TESTS)
check_AndorCamera_with_DummyCamera_SOURCES = \
    check_AndorCamera_with_DummyCamera.cpp
check_AndorCamera_with_DummyCamera_LDADD = libCImgBuffer.la

C_TESTS = check_AndorCamera_with_DummyCamera
TESTS = $(C_TESTS)
