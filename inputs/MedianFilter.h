#ifndef DSTORM_INPUT_MEDIANFILTER_H
#define DSTORM_INPUT_MEDIANFILTER_H

#include <memory>

#include "input/fwd.h"
#include "engine/Input.h"

namespace dStorm {
namespace median_filter {

std::auto_ptr<input::Link> make_link();
std::unique_ptr<input::Source<engine::ImageStack>> make_source(
        std::unique_ptr<input::Source<engine::ImageStack>> upstream,
        frame_index width, frame_index stride);

}
}

#endif
