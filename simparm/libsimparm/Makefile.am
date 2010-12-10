TESTS = testSimparm
lib_LTLIBRARIES = libsimparm.la
libsimparm_la_SOURCES = \
    ConfigSet.cc ConfigEntry.cc ConfigIO.cc ConfigObject.cc \
    ConfigEntryTrigger.cc ConfigEntryProgress.cc
library_include_HEADERS = ConfigEntry.hh ConfigSet.hh \
    ConfigIO.hh ConfigEntryNumericImpl.hh \
    ConfigEntryTypeImpl.hh ConfigObject.hh
libsimparm_la_LDFLAGS = -no-undefined -version-info 1:0:0 -lpthread

check_PROGRAMS = $(TESTS)

AM_CXXFLAGS = -Wall -Wextra 
library_includedir = $(includedir)

testSimparm_SOURCES = testSimparm.cpp
testSimparm_LDADD = -lsimparm
