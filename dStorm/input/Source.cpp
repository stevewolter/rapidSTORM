#include "Source_impl.h"

namespace dStorm {
namespace input {

BaseSource::BaseSource(simparm::Node& node, BaseSource::Flags flags)
    : node(node), flags(flags) {}


BaseSource::~BaseSource() {}

}
}
