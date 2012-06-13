#include "Status.h"
#include "Config.h"

namespace dStorm {
namespace viewer {

Status::Status(const Config& config)
: config(config),
  save("SaveImage", "Save current image")
{
}

Status::~Status() {}

void Status::attach_ui( simparm::NodeHandle name )
{
    config.attach_ui( name );
    save.attach_ui( name );
}

}
}
