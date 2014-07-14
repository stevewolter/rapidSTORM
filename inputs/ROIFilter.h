#ifndef DSTORM_INPUT_ROIFILTER_H
#define DSTORM_INPUT_ROIFILTER_H

#include <memory>
#include "engine/Image.h"
#include "input/FilterFactory.h"

namespace dStorm {
namespace ROIFilter {

std::unique_ptr<input::FilterFactory<engine::ImageStack>> create();

}
}

#endif
