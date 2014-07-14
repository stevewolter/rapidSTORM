#ifndef DSTORM_INPUT_IMAGEVECTOR_H
#define DSTORM_INPUT_IMAGEVECTOR_H

#include <memory>
#include "engine/Image.h"
#include "input/FilterFactory.h"

namespace dStorm { 
namespace input_buffer { 

std::unique_ptr<input::FilterFactory<engine::ImageStack>> create();

}
}

#endif
