noinst_LTLIBRARIES = libengine.la

library_includedir = $(includedir)/dStorm/engine
library_include_HEADERS = \
    Candidate_decl.h Candidate.h \
    CandidateTree_decl.h CandidateTree.h \
    Config_decl.h Config.h \
    Image_decl.h Image.h \
    Input_decl.h Input.h \
    Spot_decl.h Spot.h \
    SpotFinder_decl.h SpotFinder.h \
    JobInfo_decl.h JobInfo.h \
    SpotFitter_decl.h SpotFitter.h \
    SpotFitterFactory.h SpotFitterBuilder.h

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) \
    -I $(top_builddir)/..

libengine_la_SOURCES = \
    CandidateTree.cpp Config.cpp SpotFinder.cpp SpotFitterFactory.cpp Image.cpp
libengine_la_LIBADD = 
