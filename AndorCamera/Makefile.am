if BUILD_ADSTORM
    ANDORLIB=libAndorCamera.la
endif

lib_LTLIBRARIES = $(ANDORLIB) libAndorCamConfig.la
libAndorCamera_la_SOURCES = Control.cpp SDK.h SDK.cpp Error.cpp foreach.h \
    Acquisition.cpp ControlStateListener.cpp \
    Control_ConfigEntryInit.cpp \
    Control_StateTransitions.cpp \
    TemperatureMonitor.h TemperatureMonitor.cpp Error.h \
    Config.cpp
libAndorCamConfig_la_SOURCES = Config.cpp
library_includedir = $(includedir)/AndorCamera
library_include_HEADERS = AndorCamera/Control.h \
    AndorCamera/ControlReference.h \
    AndorCamera/Acquisition.h \
    AndorCamera/ControlStateListener.h \
    AndorCamera/Config.h

AM_CPPFLAGS  = -D_POSIX_SOURCE -Wall -Wextra \
    -Wno-unknown-pragmas -include $(top_builddir)/config.h \
    -include $(top_srcdir)/debug.h -I$(srcdir)/AndorCamera
AM_LDFLAGS = -no-undefined -version-info 1:0:0
libAndorCamera_la_LIBADD = $(LIBOBJS) $(LIBS) -lATMCD32D
libAndorCamConfig_la_LIBADD = $(LIBOBJS) $(LIBS)

AM_LDFLAGS += $(LIBSIMPARM_LDFLAGS)
AM_CPPFLAGS += $(LIBSIMPARM_CPPFLAGS)
libAndorCamera_la_LIBADD += $(LIBSIMPARM_LIBS)
libAndorCamConfig_la_LIBADD += $(LIBSIMPARM_LIBS)

AM_LDFLAGS += $(LIBCOMMONCPP_LDFLAGS)
AM_CPPFLAGS += $(LIBCOMMONCPP_CPPFLAGS)
libAndorCamera_la_LIBADD += $(LIBCOMMONCPP_LIBS)
