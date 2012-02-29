#include "Status.h"
#include "Config.h"
#include <simparm/ChoiceEntry_Impl.hh>

namespace dStorm {
namespace viewer {

Status::Status(const Config& config)
: config(config),
  save("SaveImage", "Save current image"),
  manager(NULL)
{
    this->config.histogramPower.setUserLevel(simparm::Object::Beginner);
}

Status::~Status() {}

void Status::registerNamedEntries( simparm::Node& name )
{
    config.registerNamedEntries( name );
    name.push_back( save );
}

void Status::add_listener( simparm::Listener& l ) {
    config.add_listener(l);
    l.receive_changes_from( save.value );
}

}
}
