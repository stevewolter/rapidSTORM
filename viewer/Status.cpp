#include "Status.h"
#include "Config.h"

namespace dStorm {
namespace viewer {

Status::Status(const Config& config)
: save("SaveImage", "Save current image"),
  manager(NULL)
{
}

Status::~Status() {}

void Status::registerNamedEntries( simparm::Node& name )
{
    config.registerNamedEntries( name );
    name.push_back( save );
}

}
}
