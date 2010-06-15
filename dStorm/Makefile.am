ACLOCAL_AMFLAGS = -I ../m4
AUTOMAKE_OPTIONS = foreign

TESTS = Pixel_test
check_PROGRAMS = Pixel_test

Pixel_test_SOURCES = Pixel_test.cpp

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
nobase_library_include_HEADERS = \
    ModuleInterface.h Localization_decl.h Localization.h \
    SizeTraits.h SizeTraits_decl.h \
    units_Eigen_traits.h unit_matrix_operators.h \
    matrix_operators.h Pixel_decl.h Pixel.h error_handler.h  \
    BaseImage_decl.h BaseImage.h \
    Image_decl.h Image.h Image_impl.h Image_iterator.h \
    ImageTraits_decl.h ImageTraits.h ImageTraits_impl.h \
    Job.h JobMaster.h UnitEntries.h log.h
image_library_includedir = $(includedir)/dStorm/image
image_library_include_HEADERS = $(addprefix image/, \
    constructors.h convert.h iterator.h minmax.h \
    )
units_library_includedir = $(includedir)/dStorm/units
units_library_include_HEADERS = \
	units/amplitude.h units/frame_count.h units/frame_rate.h \
	units/nanolength.h
unitentry_library_includedir = $(includedir)/dStorm/UnitEntries
unitentry_library_include_HEADERS = \
	$(addprefix UnitEntries/, PixelEntry.h )

libdStorm_la_SOURCES = Localization.cpp debug.h debug_print.h \
    DeferredError.h DeferredError_Impl.h DeferredError.cpp \
		       error_handler.cpp JobMaster.cpp $(RESOURCE_FILE) UnitEntries.cpp
libdStorm_la_LIBADD = outputs/liboutputs.la output/liboutput.la \
    engine/libengine.la localization_file/liblocfile.la \
    input/libinput.la helpers/libhelpers.la -lsimparm
libdStorm_la_LDFLAGS = -no-undefined -version-info @LIBDSTORM_CURRENT@:@LIBDSTORM_REVISION@:@LIBDSTORM_AGE@ -Xlinker $(RESOURCE_FILE:.rc=.o)
libdStorm_la_DEPENDENCIES = $(RESOURCE_FILE:.rc=.o) $(libdStorm_la_LIBADD)
