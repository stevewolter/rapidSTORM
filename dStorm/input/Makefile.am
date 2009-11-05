noinst_LTLIBRARIES = libinput.la 
libinput_la_SOURCES = \
    Config.cpp \
    foreach.h \
    ImageSource.cpp \
    Source.cpp ImageTraits.cpp
library_includedir = $(includedir)/dStorm/input
library_include_HEADERS = Source.h Source_impl.h Config.h Slot.h \
	Drain.h InputMethod.h \
	Traits.h ImageTraits.h \
	Buffer_decl.h Buffer.h Buffer_impl.h \
	Slot_impl.h \
	SerialImageSource.h \
	BasenameWatcher.h
AM_CPPFLAGS = -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/..
