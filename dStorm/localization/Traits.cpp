#include "Traits.h"

#include <dStorm/unit_interval.h>
#include <dStorm/engine/InputTraits.h>

namespace dStorm {
namespace input {

Traits<dStorm::Localization>::Traits() 
    : in_sequence(false) {}

Traits<dStorm::Localization>::Traits( const Traits<Localization>& i )
: input::BaseTraits(i),
  DataSetTraits(i),
  traits::Position(i),
  traits::Amplitude(i),
  traits::CovarianceMatrix(i),
  traits::TwoKernelImprovement(i),
  traits::FitResidues(i),
  traits::ImageNumber(i),
  traits::Fluorophore(i),
  traits::LocalBackground(i),
  source_traits(i.source_traits),
  in_sequence(i.in_sequence),
  repetitions(i.repetitions)
{}

Traits<dStorm::Localization>::Traits(
    const dStorm::input::Traits<dStorm::engine::ImageStack>& imageTraits )
: in_sequence(true)
{
    for (int i = 0; i < 2; ++i) {
        position().range()[i].first = 0 * si::meter;
        const image::MetaInfo<2>& im = imageTraits.plane(0).image;
        if ( im.has_resolution() ) {
            position().resolution()[i] = im.resolution(i).in_dpm();
            position().range()[i].second = (im.size[i]-1*camera::pixel) / *position().resolution()[i];
        }
    }

    image_number() = imageTraits.image_number();
}

Traits<dStorm::Localization>*
Traits<dStorm::Localization>::clone() const
    { return new Traits(*this); }

std::string Traits<dStorm::Localization>::desc() const
 { return "localization"; }

}
}
