ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign

SUBDIRS = helpers data-c++ input localization_file engine output outputs 

lib_LTLIBRARIES = libdStorm.la

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/..

if HAVE_WINDRES
    RESOURCE_FILE = windows_resources.rc
else
    RESOURCE_FILE =
endif

.rc.o : 
	$(WINDRES) -I . '$<' '$@'
windows_resources.o : config.h

library_includedir = $(includedir)/dStorm
library_include_HEADERS = \
    ModuleInterface.h Localization_decl.h Localization.h \
    SizeTraits.h SizeTraits_decl.h \
    units_Eigen_traits.h unit_matrix_operators.h \
    matrix_operators.h Pixel.h error_handler.h  \
    Job.h JobMaster.h
units_library_includedir = $(includedir)/dStorm/units
units_library_include_HEADERS = \
	units/amplitude.h units/frame_count.h units/frame_rate.h \
	units/nanolength.h

libdStorm_la_SOURCES = Localization.cpp debug.h debug_print.h \
    DeferredError.h DeferredError_Impl.h DeferredError.cpp \
		       error_handler.cpp JobMaster.cpp $(RESOURCE_FILE)
libdStorm_la_LIBADD = outputs/liboutputs.la output/liboutput.la \
    engine/libengine.la localization_file/liblocfile.la \
    input/libinput.la helpers/libhelpers.la -lsimparm
libdStorm_la_LDFLAGS = -no-undefined -version-info @LIBDSTORM_CURRENT@:@LIBDSTORM_REVISION@:@LIBDSTORM_AGE@ -Xlinker $(RESOURCE_FILE:.rc=.o)
libdStorm_la_DEPENDENCIES = $(RESOURCE_FILE:.rc=.o)
