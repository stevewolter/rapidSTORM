#ifndef DSTORM_INPUT_CHAIN_DEFAULTFILTERTYPES_H
#define DSTORM_INPUT_CHAIN_DEFAULTFILTERTYPES_H

#include <boost/mpl/vector.hpp>
#include "../../Localization_decl.h"
#include "../../engine/Image_decl.h"
#include "../../localization/record_decl.h"
#include "../../output/LocalizedImage_decl.h"
#include "../../output/LocalizedImage.h"
#include "../LocalizationTraits.h"
#include "../../ImageTraits.h"
#include "../../localization/record.h"
#include "../../output/LocalizedImage_traits.h"

namespace dStorm {
namespace input {
namespace chain {

class DefaultTypes 
: public boost::mpl::vector<
    dStorm::engine::Image, dStorm::output::LocalizedImage, dStorm::localization::Record, dStorm::Localization>
{};

}
}
}

#endif
