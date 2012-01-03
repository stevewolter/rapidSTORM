#ifndef DSTORM_INPUT_CHAIN_DEFAULTFILTERTYPES_H
#define DSTORM_INPUT_CHAIN_DEFAULTFILTERTYPES_H

#include "fwd.h"
#include <boost/mpl/vector.hpp>
#include <dStorm/Localization_decl.h>
#include <dStorm/engine/Image_decl.h>
#include <dStorm/output/LocalizedImage.h>
#include <dStorm/localization/Traits.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/localization/record.h>
#include <dStorm/output/LocalizedImage_traits.h>

namespace dStorm {
namespace input {

class DefaultTypes 
: public boost::mpl::vector<
    dStorm::engine::Image, dStorm::output::LocalizedImage>
{};

}
}

#endif
