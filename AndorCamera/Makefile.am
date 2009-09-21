if BUILD_ADSTORM
    ANDORLIB=libAndorCamera.la
else
    ANDORLIB=libAndorCamConfig.la
endif

lib_LTLIBRARIES = $(ANDORLIB)
libAndorCamera_la_SOURCES = SDK.h SDK.cpp Error.cpp foreach.h \
    Acquisition.cpp StateMachine.cpp \
    TemperatureMonitor.h TemperatureMonitor.cpp Error.h \
    Config.cpp System.cpp Camera.cpp Initialization.cpp \
    Temperature.cpp Readout.cpp AcquisitionMode.cpp \
    ShutterControl.cpp Triggering.cpp ShiftSpeedControl.cpp \
    Gain.cpp
libAndorCamConfig_la_SOURCES = Config.cpp
library_includedir = $(includedir)/AndorCamera
library_include_HEADERS = AndorCamera/Camera.h \
    AndorCamera/StateMachine.h \
    AndorCamera/System.h \
    AndorCamera/Acquisition.h \
    AndorCamera/Initialization.h \
    AndorCamera/Temperature.h \
    AndorCamera/Readout.h \
    AndorCamera/AcquisitionMode.h \
    AndorCamera/ShutterControl.h AndorCamera/Triggering.h \
    AndorCamera/ShiftSpeedControl.h \
    AndorCamera/Gain.h \
    AndorCamera/Config.h
check_PROGRAMS = testSDK

AM_CPPFLAGS  = -D_POSIX_SOURCE -Wall -Wextra \
    -Wno-unknown-pragmas -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h -I$(srcdir)/AndorCamera
AM_LDFLAGS = -no-undefined -version-info 1:0:0

LIBRARY_OBJECTS = $(LIBSIMPARM_OBJECTS) $(LIBCOMMONCPP_OBJECTS)

AM_LDFLAGS += $(LIBSIMPARM_LDFLAGS) $(LIBCOMMONCPP_LDFLAGS)
AM_CPPFLAGS += $(LIBSIMPARM_CPPFLAGS) $(LIBCOMMONCPP_CPPFLAGS)

testSDK_SOURCES = testSDK.cpp SDK.cpp Error.cpp
testSDK_LDADD = $(ANDOR_SDK_LIBS)
testSDK_CPPFLAGS = $(AM_CPPFLAGS) $(ANDOR_SDK_CPPFLAGS)

if MODIFY_ANDOR_HEADER
DYNAMICHEADERS = ATMCD32D_dynamic.h
else
DYNAMICHEADERS =
endif

libAndorCamera_la_CPPFLAGS = $(AM_CPPFLAGS) $(ANDOR_SDK_CPPFLAGS)
libAndorCamera_la_LDFLAGS = $(AM_LDFLAGS) $(ANDOR_SDK_LDFLAGS)
libAndorCamera_la_LIBADD = $(LIBOBJS) $(LIBS) $(LIBRARY_OBJECTS) $(ANDOR_SDK_LIBS)
libAndorCamera_la_DEPENDENCIES = $(LIBRARY_OBJECTS) $(ANDOR_SDK_OBJECTS)  $(DYNAMICHEADERS)
libAndorCamConfig_la_CPPFLAGS = $(AM_CPPFLAGS)
libAndorCamConfig_la_LDFLAGS = $(AM_LDFLAGS) 
libAndorCamConfig_la_LIBADD = $(LIBOBJS) $(LIBS) $(LIBADD)
libAndorCamConfig_la_DEPENDENCIES = $(LIBRARY_OBJECTS) $(ANDOR_SDK_OBJECTS)
if BUILD_STATIC_PROGRAMS
    libAndorCamera_la_LDFLAGS += -static
    libAndorCamConfig_la_LDFLAGS += -static
endif

# Need to change all functions to function types for dynamic loading. 
ATMCD32D_dynamic.h : $(ANDOR_HEADER)
	sed -e 's/\(WINAPI *\)\([A-Za-z_0-9]*\) *(/(__stdcall *\2Ptr)(/' \
	     -e 's/\(#define *EXPNETTYPE\).*$$/\1 typedef/' $< > $@
