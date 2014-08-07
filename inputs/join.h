#ifndef DSTORM_INPUT_JOIN_H
#define DSTORM_INPUT_JOIN_H

#include <memory>
#include "engine/Image.h"
#include "input/Link.h"
#include "output/LocalizedImage.h"

namespace dStorm {
namespace inputs {
namespace join {

std::unique_ptr<input::Link<engine::ImageStack>> create_image_link();
std::unique_ptr<input::Link<output::LocalizedImage>> create_localization_link();

}
}
}

#endif
