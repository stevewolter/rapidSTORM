#ifndef DSTORM_INPUT_YMIRROR_DECL_H
#define DSTORM_INPUT_YMIRROR_DECL_H

#include <memory>
#include "engine/Image.h"
#include "input/FilterFactory.h"

namespace dStorm {
namespace YMirror {

std::unique_ptr<input::FilterFactory<engine::ImageStack>> create();

}
}

#endif
