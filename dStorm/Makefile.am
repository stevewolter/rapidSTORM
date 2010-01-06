ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign

SUBDIRS = helpers data-c++ input localization_file engine output outputs 

lib_LTLIBRARIES = libdStorm.la

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/..

library_includedir = $(includedir)/dStorm
library_include_HEADERS = \
    ModuleInterface.h Localization_decl.h Localization.h \
    SizeTraits.h SizeTraits_decl.h \
    units_Eigen_traits.h unit_matrix_operators.h

libdStorm_la_SOURCES = Localization.cpp debug.h debug_print.h
libdStorm_la_LIBADD = outputs/liboutputs.la output/liboutput.la \
    engine/libengine.la localization_file/liblocfile.la \
    input/libinput.la helpers/libhelpers.la -lsimparm
libdStorm_la_LDFLAGS = -no-undefined -version-info 3:0:0 
