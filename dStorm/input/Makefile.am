noinst_LTLIBRARIES = libinput.la 
libinput_la_SOURCES = \
    Config.cpp Method.cpp \
    ImageSource.cpp \
    Source.cpp AutoInputMethod.h \
    BasenameWatcher.cpp \
    FileBasedMethod.cpp \
    ROIFilter.h ResolutionSetter.h
library_includedir = $(includedir)/dStorm/input
library_include_HEADERS = \
	AutoInputMethod.h \
	BasenameWatcher_decl.h BasenameWatcher.h \
	Buffer_decl.h Buffer.h Buffer_impl.h Buffer_impl_iterator.h \
	Config_decl.h Config.h \
	Drain_decl.h Drain.h \
	FileBasedMethod.h FileBasedMethod_impl.h \
	Method_decl.h Method.h \
	Slot_decl.h Slot.h Slot_impl.h \
	Source_decl.h Source.h Source_impl.h \
	Traits.h  \
	LocalizationTraits_decl.h LocalizationTraits.h \
	InputFilter.h InputFilter_impl.h

AM_CPPFLAGS = -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/..
