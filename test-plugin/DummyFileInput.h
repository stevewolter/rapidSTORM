#ifndef DUMMY_FILE_INPUT_H
#define DUMMY_FILE_INPUT_H

#include <dStorm/input/Source_impl.h>
#include <dStorm/input/Config.h>
#include <dStorm/engine/Image.h>
#include <dStorm/ImageTraits.h>
#include <simparm/Structure.hh>
#include <simparm/Entry.hh>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/FileInput.h>
#include <boost/signals2/connection.hpp>
#include <fstream>

namespace dummy_file_input {

std::auto_ptr< dStorm::input::chain::Link >
    make();

}

#endif
