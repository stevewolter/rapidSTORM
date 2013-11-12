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
  localization::MetaInfo<traits::PositionX>(i),
  localization::MetaInfo<traits::PositionY>(i),
  localization::MetaInfo<traits::PositionZ>(i),
  localization::MetaInfo<traits::PositionUncertaintyX>(i),
  localization::MetaInfo<traits::PositionUncertaintyY>(i),
  localization::MetaInfo<traits::PositionUncertaintyZ>(i),
  localization::MetaInfo<traits::Amplitude>(i),
  localization::MetaInfo<traits::PSFWidthX>(i),
  localization::MetaInfo<traits::PSFWidthY>(i),
  localization::MetaInfo<traits::TwoKernelImprovement>(i),
  localization::MetaInfo<traits::FitResidues>(i),
  localization::MetaInfo<traits::ImageNumber>(i),
  localization::MetaInfo<traits::Fluorophore>(i),
  localization::MetaInfo<traits::LocalBackground>(i),
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
