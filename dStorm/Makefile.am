noinst_LTLIBRARIES = libdStorm.la

library_includedir = $(includedir)/dStorm
library_include_HEADERS = \
    BasicOutputs.h \
    Candidate.h \
    CandidateTree.h \
    CarConfig.h \
    Config.h \
    FileOutputBuilder.h \
    FilterBuilder.h \
    FilterSource.h \
    Image.h \
    Input.h \
    Localization.h \
    Localizations.h \
    LocalizationFileReader.h \
    ModuleInterface.h \
    OutputBuilder.h \
    OutputFactory.h \
    Output.h \
    OutputSource.h \
    ResultRepeater.h \
    Spot.h \
    Trace.h \
    TraceReducer.h
outputs_includedir = $(includedir)/dStorm/outputs
outputs_include_HEADERS = \
    Crankshaft.h
helpers_includedir = $(includedir)/dStorm/helpers
helpers_include_HEADERS = helpers/dilation.h


AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    -I $(top_srcdir)/src

libdStorm_la_SOURCES = \
    BasicOutputs.cpp CandidateTree.cpp \
    CarConfig.cpp Config.cpp FilterSource.cpp \
    foreach.h Input.cpp Localization.cpp \
    OutputSource.cpp Trace.cpp Crankshaft.cpp \
    LocalizationFileReader.cpp Localizations.cpp \
    TraceReducers.cpp helpers/dilation.cpp
EXTRA_SOURCES = helpers/thread.h
