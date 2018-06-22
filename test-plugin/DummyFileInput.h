#ifndef DUMMY_FILE_INPUT_H
#define DUMMY_FILE_INPUT_H

#include <memory>
#include "input/fwd.h"

namespace dummy_file_input {

std::auto_ptr< dStorm::input::Link >
    make();

}

#endif
