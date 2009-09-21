TESTS = testSimparm
lib_LTLIBRARIES = libsimparm.la
libsimparm_la_SOURCES = \
    ConfigSet.cc ConfigEntry.cc ConfigIO.cc ConfigObject.cc \
    ConfigEntryTrigger.cc ConfigEntryProgress.cc \
    Node.cc Attribute.cc TreeEntry.cc
library_includedir = $(includedir)/simparm
library_include_HEADERS = \
    simparm/Node.hh simparm/Attribute.hh simparm/Object.hh \
    simparm/Entry.hh simparm/Set.hh simparm/NumericEntry.hh \
    simparm/IO.hh \
    simparm/NumericEntry_Impl.hh simparm/Entry_Impl.hh \
    simparm/ChoiceEntry_Impl.hh \
    simparm/ChoiceEntry.hh simparm/FileEntry.hh \
    simparm/Structure.hh
libsimparm_la_LDFLAGS = -no-undefined -version-info 1:0:0 -lpthread

check_PROGRAMS = $(TESTS)

AM_CPPFLAGS = -I$(srcdir)/simparm
AM_CXXFLAGS = -Wall -Wextra 

testSimparm_SOURCES = testSimparm.cpp
testSimparm_LDADD = -lsimparm
