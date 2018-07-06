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

template <typename ValueType>
struct MetaInfo {
    MetaInfo() : is_given(false) {}

    typedef boost::optional<ValueType> RangeBoundType;
    typedef std::pair< boost::optional<ValueType>, boost::optional<ValueType> > RangeType;

    static const bool has_range = true;

    bool is_given;
    boost::optional<ValueType> static_value;

    const RangeType& range() const { return range_; }
    RangeType& range() { return range_; }

    bool is_in_range( const ValueType& t) const {
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
class Traits< Localization > 
: public input::BaseTraits
{
  public:
    Traits();
    Traits* clone() const;
    std::string desc() const;

#define TRAITS_FOR_FIELD(x,n) \
  private: \
    localization::MetaInfo<typename x::ValueType> n ## _; \
  public: \
    localization::MetaInfo<typename x::ValueType>& n() { return n ## _; } \
    const localization::MetaInfo<typename x::ValueType>& n() const { return n ## _; } \
    const localization::MetaInfo<typename x::ValueType>& field(x tag) const { return n ## _; } \
    localization::MetaInfo<typename x::ValueType>& field(x tag) { return n ## _; }

    TRAITS_FOR_FIELD(localization::PositionX,position_x)
    TRAITS_FOR_FIELD(localization::PositionY,position_y)
    TRAITS_FOR_FIELD(localization::PositionZ,position_z)
    TRAITS_FOR_FIELD(localization::PositionUncertaintyX,position_uncertainty_x)
    TRAITS_FOR_FIELD(localization::PositionUncertaintyY,position_uncertainty_y)
    TRAITS_FOR_FIELD(localization::PositionUncertaintyZ,position_uncertainty_z)
    TRAITS_FOR_FIELD(localization::Amplitude,amplitude)
    TRAITS_FOR_FIELD(localization::PSFWidthX,psf_width_x)
    TRAITS_FOR_FIELD(localization::PSFWidthY,psf_width_y)
    TRAITS_FOR_FIELD(localization::TwoKernelImprovement,two_kernel_improvement)
    TRAITS_FOR_FIELD(localization::FitResidues,fit_residues)
    TRAITS_FOR_FIELD(localization::ImageNumber,image_number)
    TRAITS_FOR_FIELD(localization::Fluorophore,fluorophore)
    TRAITS_FOR_FIELD(localization::LocalBackground,local_background)
    TRAITS_FOR_FIELD(localization::CoefficientOfDetermination,coefficient_of_determination)
    TRAITS_FOR_FIELD(localization::Molecule,molecule)
#undef ACCESSORS

    bool in_sequence;
    boost::optional<int> repetitions;
};

}
}

#endif
