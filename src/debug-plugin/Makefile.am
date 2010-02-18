plugin_libdir = $(DSTORM_PLUGIN_DIR)
plugin_lib_LTLIBRARIES = libtestplugin.la
libtestplugin_la_LDFLAGS = -no-undefined -version-info 1:0:0 -module
libtestplugin_la_SOURCES = plugin.cpp SegmentationFault.h \
    MuchMemoryAllocator.h Exception.h Delayer.h \
    BasenamePrinter.h \
    Manager.h Manager.cpp \
    md5.h md5.c
libtestplugin_la_LIBADD = $(top_builddir)/../dStorm/libdStorm.la -lsimparm -lboost_thread -lpthread

AM_CPPFLAGS = -D_POSIX_SOURCE -I$(top_srcdir)/src -I$(top_srcdir)/src/locprec
