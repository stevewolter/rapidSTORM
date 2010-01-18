noinst_LTLIBRARIES = libviewer.la

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_srcdir)/../lib -I $(top_builddir)/..

libviewer_la_SOURCES = \
    ColourDisplay.cpp ColourDisplay.h ColourDisplay_impl.h \
    Display.h Display_inline.h Display_impl.h Display.cpp \
    ImageDiscretizer.h ImageDiscretizer_inline.h \
	ImageDiscretizer_impl.h ImageDiscretizer.cpp \
    ViewerConfig.h Viewer.cpp Viewer.h
