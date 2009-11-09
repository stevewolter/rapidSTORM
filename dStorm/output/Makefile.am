noinst_LTLIBRARIES = liboutput.la

library_includedir = $(includedir)/dStorm/output
library_include_HEADERS = \
    Config.h \
    FileOutputBuilder.h \
    FilterBuilder.h \
    FilterSource.h \
    Localizations.h \
    OutputBuilder.h \
    SourceFactory.h \
    Output.h \
    OutputSource.h \
    ResultRepeater.h \
    Trace_decl.h Trace.h \
    TraceReducer.h

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) \
    -I $(top_builddir)/..

liboutput_la_SOURCES = \
    Config.cpp \
    FilterSource.cpp \
    OutputSource.cpp Trace.cpp \
    Localizations.cpp \
    TraceReducers.cpp \
    SourceFactory.cpp 
liboutput_la_LIBADD = 
