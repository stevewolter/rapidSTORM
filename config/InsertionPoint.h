#ifndef DSTORM_CONFIG_INSERTIONPOINT_H
#define DSTORM_CONFIG_INSERTIONPOINT_H

#include <dStorm/input/chain/Link_decl.h>
#include <dStorm/InsertionPlace.h>
#include <memory>

namespace dStorm {

std::auto_ptr< input::chain::Link > make_insertion_place_link( InsertionPlace );

}

#endif
