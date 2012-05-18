#ifndef DSTORM_TRANSMISSIONS_SLICER_H
#define DSTORM_TRANSMISSIONS_SLICER_H

#include <dStorm/output/OutputSource.h>

namespace dStorm {
namespace slicer {

std::auto_ptr< output::OutputSource > make_output_source();

}
}
#endif
