#include "ChoiceEntry_Impl.h"
#include "ManagedChoiceEntry.h"
#include "Object.h"

namespace simparm {

struct DummyChoice {
    simparm::Object node;
    std::string getName() const { return "Dummy"; }
    void attach_ui( simparm::NodeHandle ) {}
    void detach_ui( simparm::NodeHandle ) {}
};

template class ChoiceEntry<DummyChoice>;
template class ManagedChoiceEntry<DummyChoice>;

}
