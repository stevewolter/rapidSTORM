ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign

SUBDIRS = helpers data-c++ input engine output outputs wxDisplay

lib_LTLIBRARIES = libdStorm.la

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/..

library_includedir = $(includedir)/dStorm
library_include_HEADERS = \
    ModuleInterface.h Localization.h

libdStorm_la_SOURCES = Localization.cpp
nodist_EXTRA_libdStorm_la_SOURCES = dummy.cpp
libdStorm_la_LIBADD = outputs/liboutputs.la output/liboutput.la \
    engine/libengine.la input/libinput.la \
    wxDisplay/libdStorm_display.la \
    helpers/libhelpers.la -lsimparm
libdStorm_la_LDFLAGS = -no-undefined -version-info 1:0:0 
