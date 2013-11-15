#include "dStorm/localization/Fields.h"

namespace dStorm {
namespace localization {

template <>
std::string PositionX::get_desc() { return "position in sample space in X"; }
template <>
std::string PositionX::get_shorthand() { return "posx"; }
template <>
const PositionX::ValueType PositionX::default_value
    = PositionX::ValueType::from_value(0);

template <>
std::string PositionY::get_desc() { return "position in sample space in Y"; }
template <>
std::string PositionY::get_shorthand() { return "posy"; }
template <>
const PositionY::ValueType PositionY::default_value
    = PositionY::ValueType::from_value(0);

template <>
std::string PositionZ::get_desc() { return "position in sample space in Z"; }
template <>
std::string PositionZ::get_shorthand() { return "posz"; }
template <>
const PositionZ::ValueType PositionZ::default_value = PositionZ::ValueType::from_value(0);

template <> std::string PositionUncertaintyX::get_desc() { return "position uncertainty in sample space in X"; }
template <> std::string PositionUncertaintyX::get_shorthand() { return "sigmaposx"; }
template <>
const PositionUncertaintyX::ValueType PositionUncertaintyX::default_value
    = PositionUncertaintyX::ValueType::from_value(0);

template <> std::string PositionUncertaintyY::get_desc() { return "position uncertainty in sample space in Y"; }
template <> std::string PositionUncertaintyY::get_shorthand() { return "sigmaposy"; }
template <>
const PositionUncertaintyY::ValueType PositionUncertaintyY::default_value
    = PositionUncertaintyY::ValueType::from_value(0);

template <> std::string PositionUncertaintyZ::get_desc() { return "position uncertainty in sample space in Z"; }
template <> std::string PositionUncertaintyZ::get_shorthand() { return "sigmaposz"; }
template <>
const PositionUncertaintyZ::ValueType PositionUncertaintyZ::default_value
    = PositionUncertaintyZ::ValueType::from_value(0);

std::string ImageNumber::get_desc() { return "frame number"; }
const ImageNumber::ValueType ImageNumber::default_value
    = ImageNumber::ValueType::from_value(-1);
std::string ImageNumber::get_shorthand() { return "frame"; }

std::string Amplitude::get_desc() { return "emission strength"; }
const Amplitude::ValueType Amplitude::default_value
    = Amplitude::ValueType::from_value(0);
std::string Amplitude::get_shorthand() { return "amp"; }

template <> std::string PSFWidthX::get_desc() { return "PSF FWHM in X"; }
template <>
const PSFWidthY::ValueType PSFWidthX::default_value = PSFWidthX::ValueType::from_value(0);
template <>
std::string PSFWidthX::get_shorthand() { return "psffwhmx"; }

template <>
std::string PSFWidthY::get_desc() { return "PSF FWHM in Y"; }
template <>
const PSFWidthY::ValueType PSFWidthY::default_value = PSFWidthY::ValueType::from_value(0);
template <>
std::string PSFWidthY::get_shorthand() { return "psffwhmy"; }

std::string TwoKernelImprovement::get_desc() { return "two kernel improvement"; }
const TwoKernelImprovement::ValueType TwoKernelImprovement::default_value = 0;
std::string TwoKernelImprovement::get_shorthand() { return "fishy"; }

std::string FitResidues::get_desc() { return "fit residue chi square value"; }
const FitResidues::ValueType FitResidues::default_value = 0;
std::string FitResidues::get_shorthand() { return "chisq"; }

std::string Fluorophore::get_desc() { return "index of fluorophore type"; }
const Fluorophore::ValueType Fluorophore::default_value = 0;
std::string Fluorophore::get_shorthand() { return "fluo"; }

std::string LocalBackground::get_desc() { return "local background"; }
const LocalBackground::ValueType LocalBackground::default_value
    = 0 * boost::units::camera::ad_count;
std::string LocalBackground::get_shorthand() { return "bg"; }

}
}
