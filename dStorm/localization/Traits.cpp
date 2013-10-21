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
  traits::PositionUncertainty(i),
  traits::Amplitude(i),
  traits::PSFWidth(i),
  traits::TwoKernelImprovement(i),
  traits::FitResidues(i),
  traits::ImageNumber(i),
  traits::Fluorophore(i),
  traits::LocalBackground(i),
  source_traits(i.source_traits),
  in_sequence(i.in_sequence),
  repetitions(i.repetitions)
{}

Traits<dStorm::Localization>*
Traits<dStorm::Localization>::clone() const
    { return new Traits(*this); }

std::string Traits<dStorm::Localization>::desc() const
 { return "localization"; }

}
}
