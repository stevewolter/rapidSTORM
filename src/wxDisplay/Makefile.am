noinst_LTLIBRARIES = libdStorm_display.la

check_PROGRAMS = testDisplay
TESTS = testDisplay

libdStorm_display_la_SOURCES = \
    wxManager.h wxManager.cpp App.h App.cpp \
    Window.h Window.cpp Canvas.h Canvas.cpp \
    ZoomSlider.h ZoomSlider.cpp Key.h Key.cpp \
    helpers.h ScaleBar.h ScaleBar.cpp helpers.cpp \
    wxManager_store_image.cpp SizeConvert.h

AM_CPPFLAGS = \
    -include $(top_srcdir)/../debug.h \
    -I $(top_srcdir) -I $(top_builddir)/..

testDisplay_SOURCES = test.cpp
testDisplay_LDADD = ../../dStorm/libdStorm.la libdStorm_display.la
