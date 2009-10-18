noinst_LTLIBRARIES = libdStorm_display.la

libdStorm_display_la_SOURCES = \
    Manager.cpp \
    wxManager.h wxManager.cpp App.h App.cpp \
    Window.h Window.cpp Canvas.h Canvas.cpp \
    ZoomSlider.h ZoomSlider.cpp Key.h Key.cpp \
    helpers.h ScaleBar.h ScaleBar.cpp helpers.cpp

library_includedir = $(includedir)/dStorm/display
library_include_HEADERS = Manager.h DataSource.h

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    -I $(top_srcdir)/src
