#ifndef DSTORM_INPUT_MEDIANFILTER_H
#define DSTORM_INPUT_MEDIANFILTER_H

#include <memory>

#include "engine/Image.h"
#include "engine/Input.h"
#include "input/FilterFactory.h"

namespace dStorm {
namespace median_filter {

std::unique_ptr<input::FilterFactory<engine::ImageStack>> create();
std::unique_ptr<input::Source<engine::ImageStack>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> upstream,
        frame_index width, frame_index stride);

}
}

#endif
