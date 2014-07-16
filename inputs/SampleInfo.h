#ifndef DSTORM_INPUT_SAMPLEINFO_H
#define DSTORM_INPUT_SAMPLEINFO_H

#include <memory>
#include "engine/Image.h"
#include "input/FilterFactory.h"

namespace dStorm {
namespace input {
namespace sample_info {

std::unique_ptr<input::FilterFactory<engine::ImageStack>> create();

}
}
}

#endif
