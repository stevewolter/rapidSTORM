ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign

SUBDIRS = helpers data-c++ input engine output outputs 

lib_LTLIBRARIES = libdStorm.la

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/..

library_includedir = $(includedir)/dStorm
library_include_HEADERS = \
    ModuleInterface.h Localization_decl.h Localization.h \
    SizeTraits.h SizeTraits_decl.h

libdStorm_la_SOURCES = Localization.cpp debug.h debug_print.h
libdStorm_la_LIBADD = outputs/liboutputs.la output/liboutput.la \
    engine/libengine.la input/libinput.la \
    helpers/libhelpers.la -lsimparm
libdStorm_la_LDFLAGS = -no-undefined -version-info 3:0:0 
