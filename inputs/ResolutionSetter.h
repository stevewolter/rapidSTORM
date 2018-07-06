#ifndef DSTORM_INPUT_RESOLUTIONSETTER_DECL_H
#define DSTORM_INPUT_RESOLUTIONSETTER_DECL_H

#include <memory>
#include "engine/Image.h"
#include "input/FilterFactory.h"

namespace dStorm {
namespace input {
namespace resolution {

std::unique_ptr<FilterFactory<engine::ImageStack>> create();

}
}
}

#endif
