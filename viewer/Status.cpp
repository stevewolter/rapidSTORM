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

void Status::attach_ui( simparm::NodeHandle name )
{
    config.attach_ui( name );
    save.attach_ui( name );
}

}
}
