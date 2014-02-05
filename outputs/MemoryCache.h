#ifndef DSTORM_OUTPUTS_MEMORY_CACHE_H
#define DSTORM_OUTPUTS_MEMORY_CACHE_H

#include "output/OutputSource.h"

namespace dStorm {
namespace memory_cache {

using namespace boost::units;

class Output;
class Source;

std::auto_ptr<output::OutputSource> make_output_source();

}
}

#endif
