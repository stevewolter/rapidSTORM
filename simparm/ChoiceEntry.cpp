#include "ChoiceEntry_Impl.hh"
#include "ManagedChoiceEntry.hh"
#include "Object.hh"

namespace simparm {

struct DummyChoice {
    simparm::Object node;
    std::string getName() const { return "Dummy"; }
    void attach_ui( simparm::Node& ) {}
    void detach_ui( simparm::Node& ) {}
};

template class ChoiceEntry<DummyChoice>;
template class ManagedChoiceEntry<DummyChoice>;

}
