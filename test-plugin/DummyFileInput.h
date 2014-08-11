#ifndef DUMMY_FILE_INPUT_H
#define DUMMY_FILE_INPUT_H

#include <memory>
#include "engine/Image.h"
#include "input/Link.h"

namespace dummy_file_input {

std::unique_ptr< dStorm::input::Link<dStorm::engine::ImageStack> > make();

}

#endif
