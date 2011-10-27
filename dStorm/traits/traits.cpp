#include "image_number.h"
#include "position.h"
#include "amplitude.h"
#include "covariance_matrix.h"
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

std::string Amplitude::get_ident() { return "Amplitude"; }
std::string Amplitude::get_desc() { return "emission strength"; }
const Amplitude::ValueType Amplitude::default_value
    = Amplitude::ValueType::from_value(0);
std::string Amplitude::get_shorthand() { return "amp"; }

std::string CovarianceMatrix::get_ident() { return "PSFCovarMatrix"; }
std::string CovarianceMatrix::get_desc() { return "fitted PSF covariance matrix"; }
const CovarianceMatrix::ValueType CovarianceMatrix::default_value
    = CovarianceMatrix::ValueType::Constant( CovarianceMatrix::ValueType::Scalar::from_value(0) );
template <>
const NoRange<CovarianceMatrix>::RangeType
NoRange<CovarianceMatrix>::static_range 
    = NoRange<CovarianceMatrix>::RangeType::Constant(
        NoRange<CovarianceMatrix>::BoundPair(
            boost::optional< NoRange<CovarianceMatrix>::Type >( 
                NoRange<CovarianceMatrix>::Type(0.0f * si::metre * si::metre) ),
            boost::optional< NoRange<CovarianceMatrix>::Type >()
        )
    );
std::string CovarianceMatrix::get_shorthand() { return "covar"; }

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
