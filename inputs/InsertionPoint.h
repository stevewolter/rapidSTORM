#ifndef DSTORM_CONFIG_INSERTIONPOINT_H
#define DSTORM_CONFIG_INSERTIONPOINT_H

#include "input/fwd.h"
#include "InsertionPlace.h"
#include <memory>

namespace dStorm {

std::auto_ptr< input::Link > make_insertion_place_link( InsertionPlace );

}

#endif
