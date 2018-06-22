#ifndef DSTORM_TRACE_FILTER_H
#define DSTORM_TRACE_FILTER_H

#include "output/OutputSource.h"
#include <memory>

namespace dStorm {
namespace outputs {

std::auto_ptr< output::OutputSource > make_trace_count_source();

}
}

#endif
