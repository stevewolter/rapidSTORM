#ifndef DSTORM_AVERAGEIMAGE_H
#define DSTORM_AVERAGEIMAGE_H

#include <dStorm/output/OutputSource.h>
#include <memory>

namespace dStorm {
namespace output {

std::auto_ptr< output::OutputSource > make_average_image_source();

}
}
#endif
