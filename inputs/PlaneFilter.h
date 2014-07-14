#ifndef DSTORM_INPUT_PLANEFILTER_H
#define DSTORM_INPUT_PLANEFILTER_H

#include <memory>
#include "engine/Image.h"
#include "input/FilterFactory.h"

namespace dStorm {
namespace plane_filter {

std::unique_ptr<input::FilterFactory<engine::ImageStack>> create();

}
}

#endif
