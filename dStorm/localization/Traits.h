#ifndef DSTORM_LOCALIZATION_TRAITS_H
#define DSTORM_LOCALIZATION_TRAITS_H

#include "../traits/base.h"
#include "../traits/image_number.h"
#include "../traits/position.h"
#include "../traits/position_uncertainty.h"
#include "../traits/amplitude.h"
#include "../traits/psf_width.h"
#include "../traits/two_kernel_improvement.h"
#include "../traits/residues.h"
#include "../traits/fluorophore.h"
#include "../traits/local_background.h"

#include "../engine/Image_decl.h"
#include "../input/Traits.h"
#include "../Localization_decl.h"
#include "../DataSetTraits.h"

#include <vector>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {

namespace input {
template <> class Traits<Localization>;
}

namespace input {

template <>
struct Traits< Localization > 
: public input::BaseTraits,
  public DataSetTraits,
  public traits::Position,
  public traits::PositionUncertainty,
  public traits::Amplitude,
  public traits::PSFWidth,
  public traits::TwoKernelImprovement,
  public traits::FitResidues,
  public traits::ImageNumber,
  public traits::Fluorophore,
  public traits::LocalBackground
{
    Traits();
    Traits( const Traits& );
    Traits* clone() const;
    std::string desc() const;

#define ACCESSORS(x,n) \
    x& n() { return *this; } \
    const x& n() const { return *this; }
    ACCESSORS(traits::Position,position)
    ACCESSORS(traits::PositionUncertainty,position_uncertainty)
    ACCESSORS(traits::Amplitude,amplitude)
    ACCESSORS(traits::PSFWidth,psf_width)
    ACCESSORS(traits::TwoKernelImprovement,two_kernel_improvement)
    ACCESSORS(traits::FitResidues,fit_residues)
    ACCESSORS(traits::ImageNumber,image_number)
    ACCESSORS(traits::Fluorophore,fluorophore)
    ACCESSORS(traits::LocalBackground,local_background)
#undef ACCESSORS

    typedef std::vector< boost::shared_ptr< Traits > > Sources;
    Sources source_traits;
    bool in_sequence;
    boost::optional<int> repetitions;
};

}
}

#endif
