noinst_LTLIBRARIES = libAndorCamera.la

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h \
    -I$(top_srcdir)/src

if HAVE_LIBATMCD32D
    AndorCamera_Only_lAC_Files = \
	SDK.h SDK.cpp Error.cpp foreach.h \
	Acquisition.cpp StateMachine.cpp \
	TemperatureMonitor.h TemperatureMonitor.cpp Error.h \
	System.cpp Camera.cpp Initialization.cpp \
	Temperature.cpp Readout.cpp AcquisitionMode.cpp \
	ShutterControl.cpp Triggering.cpp ShiftSpeedControl.cpp \
	Gain.cpp
else
    AndorCamera_Only_lAC_Files =
endif
libAndorCamera_la_SOURCES = Config.cpp Config.h $(AndorCamera_Only_lAC_Files)
