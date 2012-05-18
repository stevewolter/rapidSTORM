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

void Status::attach_ui( simparm::Node& name )
{
    config.attach_ui( name );
    save.attach_ui( name );
}

void Status::add_listener( simparm::Listener& l ) {
    config.add_listener(l);
    l.receive_changes_from( save.value );
}

}
}
