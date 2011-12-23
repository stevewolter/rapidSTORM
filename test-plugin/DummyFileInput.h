#ifndef DUMMY_FILE_INPUT_H
#define DUMMY_FILE_INPUT_H

#include <memory>
#include <dStorm/input/chain/Link_decl.h>

namespace dummy_file_input {

std::auto_ptr< dStorm::input::chain::Link >
    make();

}

#endif
