ACLOCAL_AMFLAGS = -I ../../m4
AUTOMAKE_OPTIONS = foreign

TESTS = check_derivatives check_plugin
check_PROGRAMS = check_derivatives check_plugin

plugin_libdir = $(libdir)/rapid2storm/plugins
plugin_lib_LTLIBRARIES = lib3d_fitter.la

lib3d_fitter_la_SOURCES = \
    Config_decl.h Config.h Config.cpp \
    Factory.h Factory.cpp \
    Exponential3D.hh Exponential3D_ParameterHelper.hh \
    Exponential3D_impl.hh Exponential3D_Derivatives.h \
    Fitter.h Fitter.cpp \
    Specialized.cpp Specialized_naive.cpp \
    plugin.cpp
lib3d_fitter_la_LDFLAGS = -no-undefined -version-info 1:0:0 -module
lib3d_fitter_la_LIBADD = $(top_builddir)/../../dStorm/libdStorm.la -lsimparm

check_derivatives_SOURCES = check_derivatives.cpp
check_plugin_SOURCES = check_plugin.cpp
check_plugin_LDADD = lib3d_fitter.la
