noinst_LTLIBRARIES = libengine.la

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_builddir)/.. -I $(top_srcdir) -I $(top_srcdir)/../lib

libengine_la_SOURCES = \
    CarConfig_decl.h CarConfig.h CarConfig.cpp \
    Car_decl.h Car.cpp Car.h Engine.cpp \
    EngineDebug.h Engine.h \
    LocalizationBuncher.cpp LocalizationBuncher.h \
    SigmaFitter.h ImageFitter.h \
    SigmaGuesser.cpp SigmaGuesser_FreeFormFitter.cpp \
    SigmaGuesser.h SpotFitter.h \
    studentPinv.c studentPinv.h \
    SpotFinder.cpp debug.h  \
    ThresholdGuesser.h ThresholdGuesser.cpp
