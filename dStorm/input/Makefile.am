noinst_LTLIBRARIES = libinput.la 
libinput_la_SOURCES = \
    Config.cpp Method.cpp \
    ImageSource.cpp \
    Source.cpp ImageTraits.cpp AutoInputMethod.h \
    BasenameWatcher.cpp \
    SerialSource.cpp \
    FileBasedMethod.cpp
library_includedir = $(includedir)/dStorm/input
library_include_HEADERS = \
	AutoInputMethod.h \
	BasenameWatcher_decl.h BasenameWatcher.h \
	Buffer_decl.h Buffer.h Buffer_impl.h \
	Config_decl.h Config.h \
	Drain_decl.h Drain.h \
	FileBasedMethod.h FileBasedMethod_impl.h \
	ImageTraits_decl.h ImageTraits.h \
	Method_decl.h Method.h \
	SerialSource.h SerialSource_impl.h \
	Slot_decl.h Slot.h Slot_impl.h \
	Source_decl.h Source.h Source_impl.h \
	Traits.h  \
	LocalizationTraits_decl.h LocalizationTraits.h

AM_CPPFLAGS = -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/..
