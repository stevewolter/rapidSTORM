#ifndef DSTORM_INPUT_LOCALIZATION_TRAITS_H
#define DSTORM_INPUT_LOCALIZATION_TRAITS_H

#include <dStorm/units/amplitude.h>
#include <dStorm/units/frame_rate.h>
#include <dStorm/units/frame_count.h>

#include <cs_units/camera/time.hpp>
#include <cs_units/camera/frame_rate.hpp>

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
    Traits() : SizeTraits<Localization::Dim>(), two_kernel_improvement_is_set(false) {}
    Traits( SizeTraits<Localization::Dim> t )
        : SizeTraits<Localization::Dim>(t), two_kernel_improvement_is_set(false) {}

    typedef amplitude AmplitudeField;
    typedef frame_rate FrameRateField;
    typedef frame_count FrameCountField;

    simparm::optional<amplitude> min_amplitude;
    simparm::optional<frame_rate> speed;
    simparm::optional<frame_count> total_frame_count;

    bool two_kernel_improvement_is_set;

    void apply_global_settings(const Config& c) {}
};

}
}

#endif
