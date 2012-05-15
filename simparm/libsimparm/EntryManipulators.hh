#ifndef SIMPARM_ENTRY_MANIPULATORS_H
#define SIMPARM_ENTRY_MANIPULATORS_H

#include "Entry.hh"
#include "AttributeChange.hh"

namespace simparm {

class VisibilityChanger {
    AttributeChange<bool> v;
  public:
    VisibilityChanger(simparm::BasicEntry& e, bool is_viewable)
        : v(e.viewable, is_viewable) {}
};

class EditabilityChanger {
    AttributeChange<bool> v;
  public:
    EditabilityChanger(simparm::BasicEntry& e, bool is_editable)
        : v(e.editable, is_editable) {}
};

class UsabilityChanger {
    AttributeChange<bool> v1, v2;
  public:
    UsabilityChanger(simparm::BasicEntry& e, bool is_usable)
        : v1(e.editable, is_usable), v2(e.viewable, is_usable) {}
};

}

#endif
