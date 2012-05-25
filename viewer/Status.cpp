#include "Status.h"
#include "Config.h"
#include <simparm/ChoiceEntry_Impl.h>

namespace dStorm {
namespace viewer {

Status::Status(const Config& config)
: config(config),
  save("SaveImage", "Save current image"),
  manager(NULL)
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
