noinst_LTLIBRARIES = libAndorCamera.la

AM_CPPFLAGS = \
    -include $(top_builddir)/config.h \
    -include $(top_srcdir)/../debug.h

if HAVE_LIBATMCD32D
    ACS = \
	SDK.h SDK.cpp Error.cpp foreach.h \
	Acquisition.cpp StateMachine.cpp StateMachine.h \
	TemperatureMonitor.h TemperatureMonitor.cpp Error.h \
	System.cpp Camera.cpp AcquisitionSwitch.cpp Initialization.cpp \
	Temperature.cpp Readout.cpp AcquisitionMode.cpp \
	ShutterControl.cpp Triggering.cpp ShiftSpeedControl.cpp \
	Gain.cpp AndorDirect.cpp AndorDirect.h \
	ViewportSelector.h ViewportSelector.cpp \
	Initialization.h AcquisitionMode.h \
	Acquisition.h Readout.h Camera.h Triggering.h CameraReference.h \
	System.h Temperature.h ShiftSpeedControl.h Gain.h ShutterControl.h \
	AndorDirect_decl.h LiveView_decl.h LiveView.h \
	LiveView.cpp EmergencyHandler.h EmergencyHandler.cpp
else
    ACS = 
endif
libAndorCamera_la_SOURCES = Config.cpp Config.h $(ACS)

C_TESTS = check_AndorCamera_with_DummyCamera
TESTS = $(C_TESTS)
check_PROGRAMS = $(C_TESTS)
check_AndorCamera_with_DummyCamera_SOURCES = \
    check_AndorCamera_with_DummyCamera.cpp
check_AndorCamera_with_DummyCamera_LDADD = ../../dStorm/libdStorm.la ../wxDisplay/libdStorm_display.la libAndorCamera.la
