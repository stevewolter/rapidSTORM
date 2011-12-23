#include "Traits.h"

#include <dStorm/ImageTraits.h>
#include <dStorm/unit_interval.h>

namespace dStorm {
namespace input {

dStorm::input::Traits<dStorm::Localization>::Traits(
    const dStorm::input::Traits<dStorm::engine::Image>& imageTraits )
: in_sequence(true)
{
    for (int i = 0; i < 2; ++i) {
        position().range()[i].first = 0 * si::meter;
        if ( imageTraits.plane(0).resolution_given_in_dpm(i) ) {
            position().resolution()[i] = imageTraits.plane(0).resolution(i)->in_dpm();
            position().range()[i].second = (imageTraits.size[i]-1*camera::pixel) / *position().resolution()[i];
        }
    }

    image_number() = imageTraits.image_number();
}

}
}
