noinst_LTLIBRARIES = libviewer.la

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_srcdir)/../lib -I $(top_builddir)/..

libviewer_la_SOURCES = \
    Colorizer_decl.h ColourDisplay.cpp ColourDisplay.h ColourDisplay_impl.h \
    Display.h Display_inline.h Display_impl.h Display.cpp \
    ImageDiscretizer.h ImageDiscretizer_inline.h \
	ImageDiscretizer_impl.h ImageDiscretizer.cpp \
    Config_decl.h Config.h Config.cpp \
    Backend_decl.h Backend.h \
    Viewer.cpp Viewer.h \
    Publisher.h HighDepth.h \
    LiveBackend_decl.h LiveBackend.h LiveBackend_impl.h LiveBackend.cpp \
    LiveCache.h LiveCache_inline.h LiveCache_impl.h LiveCache.cpp \
    TerminalBackend_decl.h TerminalBackend.h TerminalBackend_inline.h TerminalBackend_impl.h TerminalBackend.cpp \
    TerminalCache.h TerminalCache_inline.h TerminalCache_impl.h TerminalCache.cpp \
    BinnedLocalizations.cpp \
    Status_decl.h Status.h Status.cpp \
    LiveBackend_converter.h LiveBackend_converter.cpp \
    TerminalBackend_converter.h TerminalBackend_converter.cpp \
    ImageDiscretizer_converter.h
