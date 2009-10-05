noinst_LTLIBRARIES = libengine.la

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    -I $(top_srcdir)/src

libengine_la_SOURCES = \
    Car.cpp Car.h CImg_forward.h Engine.cpp \
    EngineDebug.h Engine.h GaussFitter.cpp GaussFitter.h \
    GaussFitter_impl.h GaussFitter_ResidueAnalysis.h \
    GaussFitter_selector.cpp GaussFitter_Specialized_a.cpp \
    GaussFitter_Specialized_b.cpp GaussFitter_Specialized_c.cpp \
    GaussFitter_Specialized_FreeSigma.cpp \
    GaussFitter_Specialized.h GaussFitter_Width_Invariants.cpp \
    GaussFitter_Width_Invariants.h ImageFitter.h \
    LocalizationBuncher.cpp LocalizationBuncher.h \
    SigmaFitter.h \
    SigmaGuesser.cpp SigmaGuesser_FreeFormFitter.cpp \
    SigmaGuesser.h SpotFinder.h SpotFitter.cpp SpotFitter.h \
    studentPinv.c studentPinv.h \
    Variance.h \
    spotFinders/averageSmooth.cpp \
    spotFinders/averageSmooth.h \
    spotFinders/ErosionSmoother.h \
    spotFinders/FillholeSmoother.cpp \
    spotFinders/FillholeSmoother.h \
    spotFinders/GaussSmoothing.cpp \
    spotFinders/GaussSmoothing.h \
    spotFinders/Median.cpp \
    spotFinders/MedianSmoother.h \
    spotFinders/Reconstruction.cc \
    spotFinders/Reconstruction.hh \
    spotFinders/Spalttiefpass.h \
    spotFinders/SpotFinder.cpp
