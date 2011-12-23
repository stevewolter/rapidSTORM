#ifndef DSTORM_CONFIG_INSERTIONPOINT_H
#define DSTORM_CONFIG_INSERTIONPOINT_H

#include <dStorm/input/fwd.h>
#include <dStorm/InsertionPlace.h>
#include <memory>

namespace dStorm {

std::auto_ptr< input::Link > make_insertion_place_link( InsertionPlace );

}

#endif
