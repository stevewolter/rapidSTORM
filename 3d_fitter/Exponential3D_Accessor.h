#ifndef _3D_FITTER_EXPONENTIAL3D_ACCESSOR_HH
#define _3D_FITTER_EXPONENTIAL3D_ACCESSOR_HH

#include "Exponential3D.hh"
#include <boost/units/quantity.hpp>
#include <dStorm/units/nanolength.h>
#include <cs_units/camera/length.hpp>
#include <cs_units/camera/intensity.hpp>
#include <cs_units/camera/resolution.hpp>
#include <dStorm/units/nanoresolution.h>

namespace fitpp {
namespace Exponential3D {

template <int Kernels>
struct Model<Kernels>::Accessor {
  private:
    Variables *variables;
    Constants constants;
  public:
    Accessor(Variables* variables) 
        : variables(variables) {}

    void change_variable_set(Variables* variables) 
        { this->variables = (variables); }

    template <int name, int function>
    double get() const 
        {return ParameterMap::template value<name,function>
            (*variables, constants);}
    template <int name, int function>
    void set(double value) 
        {ParameterMap::template value<name,function>
            (*variables, constants) = value;}
    template <int name>
    void set_all(double value) 
        {ParameterMap::template Parameter<name>::set_all
            (*variables, constants, value);}

    const Constants& getConstants() const { return constants; }
    Constants& getConstants()  { return constants; }

  public:
#define METHODS(param,unit,power) \
    typedef boost::units::quantity< \
        boost::units::power_typeof_helper<unit, \
            boost::units::static_rational<power> >::type,double> \
        Quantity ## param; \
    template <int Function> \
    Quantity ## param get ## param() const { \
        return Quantity ## param::from_value(get<param,Function>()); } \
    template <int Function> \
    void set ## param(Quantity ## param v) \
        { return set<param,Function>(v.value()); } \
    void set_all_ ## param(Quantity ## param v) { set_all<param>(v.value()); }
    METHODS(MeanX,cs_units::camera::length,1);
    METHODS(MeanY,cs_units::camera::length,1);
    METHODS(MeanZ,boost::units::si::nanolength,1);
    METHODS(Amplitude,cs_units::camera::intensity,1);
    METHODS(DeltaSigmaX,dStorm::nanoresolution,1);
    METHODS(DeltaSigmaY,dStorm::nanoresolution,1);
    METHODS(BestVarianceX,cs_units::camera::length,2);
    METHODS(BestVarianceY,cs_units::camera::length,2);
    METHODS(ZAtBestSigmaX,boost::units::si::nanolength,1);
    METHODS(ZAtBestSigmaY,boost::units::si::nanolength,1);
#undef METHODS
    typedef boost::units::quantity<cs_units::camera::intensity,double>
        QuantityShift;
    QuantityShift getShift() const 
        { return QuantityShift::from_value( get<Shift,0>() ); }
    void setShift(QuantityShift v) { return set<Shift,0>(v.value()); }

    template <int Function>
    Eigen::Vector3d getPosition() const {
        Eigen::Vector3d rv;
        rv.x() = double(getMeanX<Function>() / cs_units::camera::pixel);
        rv.y() = double(getMeanY<Function>() / cs_units::camera::pixel);
        rv.z() = double(getMeanZ<Function>() / boost::units::si::nanometre);
        return rv;
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

}
}

#endif
