#include "Config.h"
#include "doc/help/context.h"
#include "fitter/MarquardtConfig_impl.h"
#include "fitter/residue_analysis/Config_impl.h"
#include <simparm/UnitEntry_Impl.hh>

namespace simparm {
template class simparm::UnitEntry<cs_units::camera::resolution, float>;
}

namespace dStorm {
namespace fitter {
namespace residue_analysis {
class Config;
}
}
}

namespace dStorm {
namespace gauss_3d_fitter {

using namespace simparm;

std::string names[] = { "Holtzer", "Zhuang" };
std::string descs[] = { " Holtzer model", "parabolic model" };

template <int Widening>
Config<Widening>::Config() 
: MarquardtConfig("3DFitter" + names[Widening], ("3D cylinder lens fit with " + descs[Widening])),
  z_distance("ZDistance", "Distance between X and Y foci"),
  z_range("ZRange", "Maximum sensible Z distance from equifocused plane", 1000 * boost::units::si::nanometre),
  defocus_constant_x("XDefocusConstant", "Speed of PSF std. dev. growth in X"),
  defocus_constant_y("YDefocusConstant", "Speed of PSF std. dev. growth in Y")
{
}

template <int Widening>
Config<Widening>::~Config() {
}

template <int Widening>
void Config<Widening>::registerNamedEntries() 
{
    push_back(marquardtStartLambda);
    push_back(maximumIterationSteps);
    push_back(negligibleStepLength);
    push_back(successiveNegligibleSteps);
    push_back(z_distance);
    push_back(z_range);
    push_back(defocus_constant_x);
    push_back(defocus_constant_y);
    fitter::residue_analysis::Config::registerNamedEntries(*this);
}

template class Config<fitpp::Exponential3D::Zhuang>;
template class Config<fitpp::Exponential3D::Holtzer>;

}
}

namespace simparm {
template class simparm::UnitEntry< fitpp::Exponential3D::ConstantTypes<fitpp::Exponential3D::Zhuang>::ResolutionUnit, float >;
}

namespace boost {
namespace units {

std::string name_string(const fitpp::Exponential3D::ConstantTypes<fitpp::Exponential3D::Zhuang>::ResolutionUnit&)
    { return "pixels per square nanometre"; }
std::string symbol_string(const fitpp::Exponential3D::ConstantTypes<fitpp::Exponential3D::Holtzer>::ResolutionUnit&)
    { return "px nm^-2"; }

}
}

