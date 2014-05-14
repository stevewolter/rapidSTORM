#ifndef DSTORM_LOCALIZATION_TRAITS_H
#define DSTORM_LOCALIZATION_TRAITS_H

#include "localization/Fields.h"

#include "engine/Image_decl.h"
#include "input/Traits.h"
#include "Localization_decl.h"

#include <vector>
#include <boost/optional/optional.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>

namespace dStorm {

namespace localization {

template <typename Tag>
struct MetaInfo {
    MetaInfo() : is_given(false) {}
    MetaInfo& operator=(const Tag& tag) {
        static_cast<Tag&>(*this) = tag;
        return *this;
    }

    typedef boost::optional<typename Tag::ValueType> RangeBoundType;
    typedef std::pair< boost::optional<typename Tag::ValueType>, boost::optional<typename Tag::ValueType> > RangeType;

    static const bool has_range = true;

    bool is_given;
    boost::optional<typename Tag::ValueType> static_value;

    const RangeType& range() const { return range_; }
    RangeType& range() { return range_; }

    bool is_in_range( const typename Tag::ValueType& t) const {
      return range_.first <= t && range_.second >= t;
    }

    RangeBoundType lower_limits() const { return range_.first; }
    RangeBoundType upper_limits() const { return range_.second; }
    RangeBoundType width() const { return range_.second - range_.first; }

  private:
    RangeType range_;
};

}

namespace input {

template <>
struct Traits< Localization > 
: public input::BaseTraits,
  public localization::MetaInfo<localization::PositionX>,
  public localization::MetaInfo<localization::PositionY>,
  public localization::MetaInfo<localization::PositionZ>,
  public localization::MetaInfo<localization::PositionUncertaintyX>,
  public localization::MetaInfo<localization::PositionUncertaintyY>,
  public localization::MetaInfo<localization::PositionUncertaintyZ>,
  public localization::MetaInfo<localization::Amplitude>,
  public localization::MetaInfo<localization::PSFWidth<0>>,
  public localization::MetaInfo<localization::PSFWidth<1>>,
  public localization::MetaInfo<localization::TwoKernelImprovement>,
  public localization::MetaInfo<localization::FitResidues>,
  public localization::MetaInfo<localization::ImageNumber>,
  public localization::MetaInfo<localization::Fluorophore>,
  public localization::MetaInfo<localization::LocalBackground>,
  public localization::MetaInfo<localization::CoefficientOfDetermination>
{
    Traits();
    Traits( const Traits& );
    Traits* clone() const;
    std::string desc() const;

#define ACCESSORS(x,n) \
    localization::MetaInfo<x>& n() { return *this; } \
    const localization::MetaInfo<x>& n() const { return *this; } \
    const localization::MetaInfo<x>& field(x tag) const { return *this; } \
    localization::MetaInfo<x>& field(x tag) { return *this; }

    ACCESSORS(localization::PositionX,position_x)
    ACCESSORS(localization::PositionY,position_y)
    ACCESSORS(localization::PositionZ,position_z)
    ACCESSORS(localization::PositionUncertaintyX,position_uncertainty_x)
    ACCESSORS(localization::PositionUncertaintyY,position_uncertainty_y)
    ACCESSORS(localization::PositionUncertaintyZ,position_uncertainty_z)
    ACCESSORS(localization::Amplitude,amplitude)
    ACCESSORS(localization::PSFWidthX,psf_width_x)
    ACCESSORS(localization::PSFWidthY,psf_width_y)
    ACCESSORS(localization::TwoKernelImprovement,two_kernel_improvement)
    ACCESSORS(localization::FitResidues,fit_residues)
    ACCESSORS(localization::ImageNumber,image_number)
    ACCESSORS(localization::Fluorophore,fluorophore)
    ACCESSORS(localization::LocalBackground,local_background)
    ACCESSORS(localization::CoefficientOfDetermination,coefficient_of_determination)
#undef ACCESSORS

    typedef std::vector< boost::shared_ptr< Traits > > Sources;
    Sources source_traits;
    bool in_sequence;
    boost::optional<int> repetitions;
};

}
}

#endif
