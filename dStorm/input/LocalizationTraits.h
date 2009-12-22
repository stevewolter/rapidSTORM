#ifndef DSTORM_INPUT_LOCALIZATION_TRAITS_H
#define DSTORM_INPUT_LOCALIZATION_TRAITS_H

#include "Traits.h"
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Input.h>
#include <dStorm/Localization.h>

namespace dStorm {
namespace input {

template <>
class Traits< Localization > 
: public SizeTraits<Localization::Dim>
{
  public:
    Traits() : SizeTraits<Localization::Dim>() {}
    Traits( SizeTraits<Localization::Dim> t )
        : SizeTraits<Localization::Dim>(t) {}

    int imageNumber;

    void apply_global_settings(const Config& c) {}
};

}
}

#endif
