#include "viewer/Status.h"
#include "viewer/Config.h"
#include "simparm/dummy_ui/fwd.h"

namespace dStorm {
namespace viewer {

Status::Status(const Config& config)
: config(config),
  save("SaveImage", "Save current image")
{
    this->config.attach_ui( simparm::dummy_ui::make_node() );
}

Status::~Status() {}

void Status::attach_ui( simparm::NodeHandle name )
{
    config.attach_ui( name );
    save.attach_ui( name );
}

}
}
