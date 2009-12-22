noinst_LTLIBRARIES = libengine.la

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_builddir)/.. -I $(top_srcdir) -I $(top_srcdir)/../lib

libengine_la_SOURCES = \
    CarConfig.h CarConfig.cpp Car.cpp Car.h Engine.cpp \
    EngineDebug.h Engine.h \
    GaussFitter_decl.h GaussFitter.h GaussFitter_impl.h GaussFitter.cpp \
    GaussFitter_ResidueAnalysis.h \
    GaussFitter_selector.cpp GaussFitter_Specialized_a.cpp \
    GaussFitter_Specialized_b.cpp GaussFitter_Specialized_c.cpp \
    GaussFitter_Specialized_FreeSigma.cpp \
    GaussFitter_Specialized.h GaussFitter_Width_Invariants.cpp \
    GaussFitter_Width_Invariants.h ImageFitter.h \
    LocalizationBuncher.cpp LocalizationBuncher.h \
    SigmaFitter.h \
    SigmaGuesser.cpp SigmaGuesser_FreeFormFitter.cpp \
    SigmaGuesser.h SpotFitter.cpp SpotFitter.h \
    studentPinv.c studentPinv.h \
    SpotFinder.cpp debug.h
