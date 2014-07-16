#ifndef DSTORM_INPUT_FILTERFACTORYLINK_H
#define DSTORM_INPUT_FILTERFACTORYLINK_H

#include "input/FilterFactory.h"
#include "input/Link.h"

namespace dStorm {
namespace engine { class ImageStack; }
namespace output { class LocalizedImage; }
namespace input {

std::unique_ptr<Link> CreateLink(
    std::unique_ptr<FilterFactory<engine::ImageStack, engine::ImageStack>> filter);
std::unique_ptr<Link> CreateLink(
    std::unique_ptr<FilterFactory<engine::ImageStack, output::LocalizedImage>> filter);
std::unique_ptr<Link> CreateLink(
    std::unique_ptr<FilterFactory<output::LocalizedImage, output::LocalizedImage>> filter);

}
}

#endif
