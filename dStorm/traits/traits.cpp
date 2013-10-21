#include "image_number.h"
#include "position.h"
#include "position_uncertainty.h"
#include "amplitude.h"
#include "psf_width.h"
#include "two_kernel_improvement.h"
#include "residues.h"
#include "fluorophore.h"
#include "local_background.h"

namespace dStorm {
namespace traits {

ImageNumber::ImageNumber() { 
    range().first = 0 * camera::frame;
}
std::string ImageNumber::get_ident() { return "ImageNumber"; }
std::string ImageNumber::get_desc() { return "frame number"; }
const ImageNumber::ValueType ImageNumber::default_value
    = ImageNumber::ValueType::from_value(-1);
std::string ImageNumber::get_shorthand() { return "frame"; }

std::string Position::get_ident() { return "Position"; }
std::string Position::get_desc() { return "position in sample space"; }
std::string Position::get_shorthand() { return "pos"; }
const Position::ValueType Position::default_value
    = Position::ValueType::Constant( Position::ValueType::Scalar::from_value(0) );

template <> std::string PositionUncertaintyX::get_ident() { return "Position-0-0-uncertainty"; }
template <> std::string PositionUncertaintyX::get_desc() { return "position uncertainty in sample space in X"; }
template <> std::string PositionUncertaintyX::get_shorthand() { return "sigmaposx"; }
template <>
const PositionUncertaintyX::ValueType PositionUncertaintyX::default_value
    = PositionUncertaintyX::ValueType::from_value(0);
template <>
const PositionUncertaintyX::RangeType
NoRange<PositionUncertaintyX>::static_range 
    = PositionUncertaintyX::BoundPair();

template <> std::string PositionUncertaintyY::get_ident() { return "Position-1-0-uncertainty"; }
template <> std::string PositionUncertaintyY::get_desc() { return "position uncertainty in sample space in Y"; }
template <> std::string PositionUncertaintyY::get_shorthand() { return "sigmaposy"; }
template <>
const PositionUncertaintyY::ValueType PositionUncertaintyY::default_value
    = PositionUncertaintyY::ValueType::from_value(0);
template <>
const PositionUncertaintyY::RangeType
NoRange<PositionUncertaintyY>::static_range 
    = PositionUncertaintyY::BoundPair();

template <> std::string PositionUncertaintyZ::get_ident() { return "Position-2-0-uncertainty"; }
template <> std::string PositionUncertaintyZ::get_desc() { return "position uncertainty in sample space in Z"; }
template <> std::string PositionUncertaintyZ::get_shorthand() { return "sigmaposz"; }
template <>
const PositionUncertaintyZ::ValueType PositionUncertaintyZ::default_value
    = PositionUncertaintyZ::ValueType::from_value(0);
template <>
const PositionUncertaintyZ::RangeType
NoRange<PositionUncertaintyZ>::static_range 
    = PositionUncertaintyZ::BoundPair();

std::string Amplitude::get_ident() { return "Amplitude"; }
std::string Amplitude::get_desc() { return "emission strength"; }
const Amplitude::ValueType Amplitude::default_value
    = Amplitude::ValueType::from_value(0);
std::string Amplitude::get_shorthand() { return "amp"; }

template <> std::string PSFWidthX::get_ident() { return "PSFWidth-0"; }
template <> std::string PSFWidthX::get_desc() { return "PSF FWHM in X"; }
template <>
const PSFWidthY::ValueType PSFWidthX::default_value = PSFWidthX::ValueType::from_value(0);
template <>
const NoRange<PSFWidthX>::RangeType
NoRange<PSFWidthX>::static_range 
    = NoRange<PSFWidthX>::BoundPair(
            boost::optional< NoRange<PSFWidthX>::Type >( 
                NoRange<PSFWidthX>::Type(0.0f * si::metre) ),
            boost::optional< NoRange<PSFWidthX>::Type >()
    );
template <>
std::string PSFWidthX::get_shorthand() { return "psffwhmx"; }

template <>
std::string PSFWidthY::get_ident() { return "PSFWidth-1"; }
template <>
std::string PSFWidthY::get_desc() { return "PSF FWHM in Y"; }
template <>
const PSFWidthY::ValueType PSFWidthY::default_value = PSFWidthY::ValueType::from_value(0);
template <>
const NoRange<PSFWidthY>::RangeType
NoRange<PSFWidthY>::static_range 
    = NoRange<PSFWidthY>::BoundPair(
            boost::optional< NoRange<PSFWidthY>::Type >( 
                NoRange<PSFWidthY>::Type(0.0f * si::metre) ),
            boost::optional< NoRange<PSFWidthY>::Type >()
    );
template <>
std::string PSFWidthY::get_shorthand() { return "psffwhmy"; }

std::string TwoKernelImprovement::get_ident() { return "TwoKernelImprovement"; }
std::string TwoKernelImprovement::get_desc() { return "two kernel improvement"; }
const TwoKernelImprovement::ValueType TwoKernelImprovement::default_value = 0;
template <>
const TwoKernelImprovement::RangeType
NoRange<TwoKernelImprovement>::static_range 
    = TwoKernelImprovement::BoundPair(
        boost::optional< TwoKernelImprovement::ValueType >(0), boost::optional< TwoKernelImprovement::ValueType >(1) );
std::string TwoKernelImprovement::get_shorthand() { return "fishy"; }

std::string FitResidues::get_ident() { return "FitResidues"; }
std::string FitResidues::get_desc() { return "fit residue chi square value"; }
const FitResidues::ValueType FitResidues::default_value = 0;
template <>
const FitResidues::RangeType
NoRange<FitResidues>::static_range 
    = FitResidues::BoundPair();
std::string FitResidues::get_shorthand() { return "chisq"; }

std::string Fluorophore::get_ident() { return "Fluorophore"; }
std::string Fluorophore::get_desc() { return "index of fluorophore type"; }
const Fluorophore::ValueType Fluorophore::default_value = 0;
template <>
const Fluorophore::RangeType
NoRange<Fluorophore>::static_range 
    = Fluorophore::BoundPair();
std::string Fluorophore::get_shorthand() { return "fluo"; }

std::string LocalBackground::get_ident() { return "LocalBackground"; }
std::string LocalBackground::get_desc() { return "local background"; }
const LocalBackground::ValueType LocalBackground::default_value
    = 0 * boost::units::camera::ad_count;
std::string LocalBackground::get_shorthand() { return "bg"; }

std::string axis_name(int index) {
    switch (index) {
        case 0: return "x";
        case 1: return "y";
        case 2: return "z";
        case 3: return "a";
        default: throw std::runtime_error("No axis name for axis " + std::string(1, '0' + index));
    }
}

}
}
