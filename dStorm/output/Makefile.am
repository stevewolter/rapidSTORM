noinst_LTLIBRARIES = liboutput.la

library_includedir = $(includedir)/dStorm/output
library_include_HEADERS = \
    Capabilities.h \
    Config.h \
    FileOutputBuilder.h \
    FilterBuilder.h FilterBuilder_impl.h \
    FilterSource.h \
    Localizations.h \
    OutputBuilder.h OutputBuilder_impl.h \
    SourceFactory_decl.h SourceFactory.h \
    Output.h \
    OutputSource.h \
    ResultRepeater.h \
    Trace_decl.h Trace.h \
    Traits_decl.h Traits.h \
    TraceReducer.h

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) \
    -I $(top_builddir)/..

liboutput_la_SOURCES = \
    debug.h \
    Config.cpp \
    FilterSource.cpp \
    OutputSource.cpp Trace.cpp \
    Traits.cpp \
    Localizations.cpp \
    TraceReducers.cpp \
    SourceFactory.cpp 
liboutput_la_LIBADD = 
