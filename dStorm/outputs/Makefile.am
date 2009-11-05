noinst_LTLIBRARIES = liboutputs.la

library_includedir = $(includedir)/dStorm/outputs
library_include_HEADERS = BinnedLocalizations.h LocalizationList.h TraceFilter.h Crankshaft.h

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/..

liboutputs_la_SOURCES = \
    BinnedLocalizations.cpp LocalizationList.cpp TraceFilter.cpp Crankshaft.cpp \
    BinnedLocalizations_impl.h
