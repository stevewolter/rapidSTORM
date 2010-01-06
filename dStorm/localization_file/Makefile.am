noinst_LTLIBRARIES = liblocfile.la 
liblocfile_la_SOURCES = \
    fields.cpp reader.cpp known_fields.cpp
liblocfile_la_LIBADD = -lberghen_xmlParser
library_includedir = $(includedir)/dStorm/localization_file
library_include_HEADERS = \
	fields_decl.h fields.h fields_impl.h \
	known_fields_decl.h known_fields.h \
	reader.h

AM_CPPFLAGS = -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/..
