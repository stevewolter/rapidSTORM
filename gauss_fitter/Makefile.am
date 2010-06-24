ACLOCAL_AMFLAGS = -I ../../m4
AUTOMAKE_OPTIONS = foreign subdir-objects

noinst_LTLIBRARIES = libgauss_fitter.la

libgauss_fitter_la_SOURCES = \
    Config_decl.h Config.h Config.cpp \
    Factory.h Factory.cpp \
    no_analysis/main.cpp no_analysis/main.h \
    no_analysis/fitters_uncorrelated.cpp \
    no_analysis/fitters_correlated.cpp \
    residue_analysis/main.cpp \
    residue_analysis/main.h residue_analysis/impl.h \
    residue_analysis/fitters_uncorrelated_0.cpp \
    residue_analysis/fitters_uncorrelated_1.cpp \
    residue_analysis/fitters_correlated.cpp
