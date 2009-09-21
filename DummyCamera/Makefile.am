lib_LTLIBRARIES = libdummy_andor_camera.la
libdummy_andor_camera_la_SOURCES = dummycamera.cpp 
libdummy_andor_camera_la_LDFLAGS = -no-undefined -version-info 1:0:0 
EXTRA_DIST=ATMCD32D.H

AM_CPPFLAGS = -D_GNU_SOURCE
AM_CPPFLAGS += -Wall -Wextra \
    -include $(top_builddir)/config.h -include $(top_srcdir)/debug.h
