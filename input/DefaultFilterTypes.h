#ifndef DSTORM_INPUT_CHAIN_DEFAULTFILTERTYPES_H
#define DSTORM_INPUT_CHAIN_DEFAULTFILTERTYPES_H

#include "input/fwd.h"
#include <boost/mpl/vector.hpp>
#include "engine/Image_decl.h"
#include "output/LocalizedImage.h"
#include "engine/InputTraits.h"
#include "output/LocalizedImage_traits.h"

namespace dStorm {
namespace input {

class DefaultTypes 
: public boost::mpl::vector<
    dStorm::engine::ImageStack, dStorm::output::LocalizedImage>
{};

}
}

#endif
