#ifndef DSTORM_INPUT_LOCALIZATION_TRAITS_H
#define DSTORM_INPUT_LOCALIZATION_TRAITS_H

#include "Traits.h"
#include <dStorm/engine/Image.h>
#include <dStorm/Localization.h>

namespace dStorm {
namespace input {

template <>
class Traits< Localization > 
    : public engine::InputTraits 
{
  public:
    int imageNumber;

    Traits() {}
    Traits( const engine::InputTraits& imageTraits, int imageNumber )
        : engine::InputTraits(imageTraits), imageNumber(imageNumber) {}
};

}
}

#endif
