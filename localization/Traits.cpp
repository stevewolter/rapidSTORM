#include "localization/Traits.h"

#include "unit_interval.h"
#include "engine/InputTraits.h"

namespace dStorm {
namespace input {

Traits<dStorm::Localization>::Traits() 
    : in_sequence(false) {}

Traits<dStorm::Localization>::Traits( const Traits<Localization>& i )
: input::BaseTraits(i),
  localization::MetaInfo<localization::PositionX>(i),
  localization::MetaInfo<localization::PositionY>(i),
  localization::MetaInfo<localization::PositionZ>(i),
  localization::MetaInfo<localization::PositionUncertaintyX>(i),
  localization::MetaInfo<localization::PositionUncertaintyY>(i),
  localization::MetaInfo<localization::PositionUncertaintyZ>(i),
  localization::MetaInfo<localization::Amplitude>(i),
  localization::MetaInfo<localization::PSFWidthX>(i),
  localization::MetaInfo<localization::PSFWidthY>(i),
  localization::MetaInfo<localization::TwoKernelImprovement>(i),
  localization::MetaInfo<localization::FitResidues>(i),
  localization::MetaInfo<localization::ImageNumber>(i),
  localization::MetaInfo<localization::Fluorophore>(i),
  localization::MetaInfo<localization::LocalBackground>(i),
  localization::MetaInfo<localization::CoefficientOfDetermination>(i),
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
