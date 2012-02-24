#ifndef DSTORM_INPUT_CHAIN_DEFAULTFILTERTYPES_H
#define DSTORM_INPUT_CHAIN_DEFAULTFILTERTYPES_H

#include "fwd.h"
#include <boost/mpl/vector.hpp>
#include <dStorm/engine/Image_decl.h>
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/engine/InputTraits.h>
#include <dStorm/output/LocalizedImage_traits.h>

namespace dStorm {
namespace input {

class DefaultTypes 
: public boost::mpl::vector<
    dStorm::engine::ImageStack, dStorm::output::LocalizedImage>
{};

}
}

#endif
