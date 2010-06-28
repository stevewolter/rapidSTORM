ACLOCAL_AMFLAGS = -I ../../m4
AUTOMAKE_OPTIONS = foreign

plugin_libdir = $(libdir)/rapid2storm/plugins
plugin_lib_LTLIBRARIES = lib3d_fitter.la

lib3d_fitter_la_SOURCES = \
    Config_decl.h Config.h Config.cpp \
    Factory.h Factory.cpp \
    Exponential3D.hh Exponential3D_ParameterHelper.hh \
    Exponential3D_impl.hh Exponential3D_Derivatives.h \
    Fitter.h Fitter.cpp \
    Specialized.cpp \
    plugin.cpp
lib3d_fitter_la_LDFLAGS = -no-undefined -version-info 1:0:0 -module
lib3d_fitter_la_LIBADD = $(top_builddir)/../../dStorm/libdStorm.la -lsimparm
