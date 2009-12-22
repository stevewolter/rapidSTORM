#include "SourceFactory.h"
#include <stdexcept>

#include "debug.h"

namespace dStorm {
namespace output {
    SourceFactory::SourceFactory(simparm::Node& node)
        : node(node) {}
    SourceFactory::SourceFactory
        (simparm::Node& node, const SourceFactory& o) : node(node) {}
    SourceFactory::~SourceFactory() {
        DEBUG("Destructor");
    }

}
}
