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
}

Status::~Status() {}

void Status::registerNamedEntries( simparm::Node& name )
{
    config.registerNamedEntries( name );
    name.push_back( save );
}

}
}
