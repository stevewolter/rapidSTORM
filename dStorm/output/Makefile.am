noinst_LTLIBRARIES = liboutput.la

library_includedir = $(includedir)/dStorm/output
library_include_HEADERS = \
    BasicOutputs.h \
    FileOutputBuilder.h \
    FilterBuilder.h \
    FilterSource.h \
    Localization.h \
    Localizations.h \
    LocalizationFileReader.h \
    OutputBuilder.h \
    OutputFactory.h \
    Output.h \
    OutputSource.h \
    ResultRepeater.h \
    Trace.h \
    TraceReducer.h

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) \
    -I $(top_builddir)/..

liboutput_la_SOURCES = \
    BasicOutputs.cpp \
    FilterSource.cpp \
    foreach.h Localization.cpp \
    OutputSource.cpp Trace.cpp \
    LocalizationFileReader.cpp Localizations.cpp \
    TraceReducers.cpp \
    OutputFactory.cpp 
liboutput_la_LIBADD = 
EXTRA_DIST = helpers/thread.h
