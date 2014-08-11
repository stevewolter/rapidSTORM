#ifndef DSTORM_CONFIG_INPUTBASE_H
#define DSTORM_CONFIG_INPUTBASE_H

#include <memory>
#include "engine/Image.h"
#include "input/Link.h"
#include "output/LocalizedImage.h"

namespace dStorm {

std::unique_ptr< input::Link<engine::ImageStack> > make_image_input_base(
    std::unique_ptr<input::Link<engine::ImageStack>> upstream);
std::unique_ptr< input::Link<output::LocalizedImage> > make_localization_input_base(
    std::unique_ptr<input::Link<output::LocalizedImage>> upstream);

}

#endif
