UNIT_TESTS = testChoice test_units test_Link test_optional testSimparm  

# AUTOMAKE_OPTIONS = dejagnu
# DEJATOOL = simparm

noinst_LTLIBRARIES = libsimparm.la
libsimparm_la_SOURCES = \
    Callback.cc \
    ConfigSet.cc ConfigEntry.cc ConfigIO.cc ConfigObject.cc \
    ConfigEntryTrigger.cc ConfigEntryProgress.cc \
    Node.cc Attribute.cc TreeEntry.cc Node_Callback.cc \
    Node_debug.cc FileEntry.cc Message.cpp \
    ChoiceEntry.cpp \
    stl_helpers.hh Node_debug.hh Node_Pimpl.hh \
    debug.hh Callback.hh Node.hh Attribute.hh Object.hh \
    Entry_decl.hh Entry.hh Set.hh IO.hh \
    VectorEntry.hh \
    Entry_Impl.hh ChoiceEntry_Impl.hh ManagedChoiceEntry.hh \
    ChoiceEntry.hh FileEntry.hh TreeEntry.hh \
    TriggerEntry.hh ProgressEntry.hh iostream.hh \
    Link.hh Link_impl.hh Link_iterator.hh \
    AttributeChange.hh EntryManipulators.hh \
    Node_decl.hh Eigen.hh \
    Message_decl.hh Message.hh Menu.hh \
    Eigen_decl.hh default_value.hh IncrementWatcher.hh BoostUnits.hh \
    MinMaxWatcher.hh MinMaxWatcher_decl.hh BoostOptional.hh Attributes.hh \
    ObjectChoice.hh command_line.hh BoostSignal.hh NodeHandle.hh
libsimparm_la_LDFLAGS = -lpthread 

check_PROGRAMS = $(UNIT_TESTS) test_eigen testSimparm

AM_CPPFLAGS = -I$(srcdir)/simparm
AM_CXXFLAGS = -Wall -Wextra 

testChoice_SOURCES = testChoice.cpp
testChoice_LDADD = libsimparm.la
testSimparm_SOURCES = testSimparm.cpp
testSimparm_LDADD = libsimparm.la
test_optional_SOURCES = test_optional.cpp
test_optional_LDADD = libsimparm.la
test_Link_SOURCES = test_Link.cpp
test_units_SOURCES = test_units.cpp
test_units_LDADD = libsimparm.la
test_eigen_SOURCES = test_eigen.cpp
test_eigen_LDADD = libsimparm.la
