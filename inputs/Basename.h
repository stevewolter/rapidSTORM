#ifndef DSTORM_INPUT_BASENAME_DECL_H
#define DSTORM_INPUT_BASENAME_DECL_H

#include <memory>
#include "engine/Image.h"
#include "input/Link.h"
#include "output/LocalizedImage.h"

namespace dStorm {
namespace basename_input_field {

std::unique_ptr<input::Link<engine::ImageStack>> makeImageLink(
    std::unique_ptr<input::Link<engine::ImageStack>> upstream);
std::unique_ptr<input::Link<output::LocalizedImage>> makeLocalizationLink(
    std::unique_ptr<input::Link<output::LocalizedImage>> upstream);

}
}

#endif
