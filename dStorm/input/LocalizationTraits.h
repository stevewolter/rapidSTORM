#ifndef DSTORM_INPUT_LOCALIZATION_TRAITS_H
#define DSTORM_INPUT_LOCALIZATION_TRAITS_H

#include "Traits.h"
#include <dStorm/Localization.h>
#include <dStorm/SizeTraits.h>
#include <dStorm/engine/Image.h>
#include <dStorm/engine/Input.h>
#include <dStorm/Localization.h>
#include <simparm/optional.hh>

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

    simparm::optional<
        quantity<camera::intensity> > min_amplitude;

    simparm::optional<
        quantity<camera::frame_rate> > frame_length;

    simparm::optional<frame_count> total_frame_count;

    void apply_global_settings(const Config& c) {}
};

}
}

#endif
