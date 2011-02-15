#ifndef _3D_FITTER_EXPONENTIAL3D_ACCESSOR_HH
#define _3D_FITTER_EXPONENTIAL3D_ACCESSOR_HH

#include "Exponential3D.hh"
#include "ConstantTypes.h"
#include <boost/units/quantity.hpp>
#include <dStorm/units/nanolength.h>
#include <boost/units/systems/camera/length.hpp>
#include <boost/units/systems/camera/intensity.hpp>
#include <boost/units/systems/camera/resolution.hpp>
#include <dStorm/units/nanoresolution.h>

namespace fitpp {
namespace Exponential3D {

template <int Kernels,int Widening>
struct Model<Kernels,Widening>::Accessor {
  private:
    Variables *variables;
    Constants constants;

    typedef ConstantTypes<Widening> TypeHelper;
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
#define METHODS(param,unit) \
    typedef boost::units::quantity<unit ,double> \
        Quantity ## param; \
    template <int Function> \
    Quantity ## param get ## param() const { \
        return Quantity ## param::from_value(get<param,Function>()); } \
    template <int Function> \
    void set ## param(Quantity ## param v) \
        { return set<param,Function>(v.value()); } \
    void set_all_ ## param(Quantity ## param v) { set_all<param>(v.value()); }
    METHODS(MeanX,boost::units::camera::length);
    METHODS(MeanY,boost::units::camera::length);
    METHODS(MeanZ,boost::units::si::nanolength);
    METHODS(Amplitude,boost::units::camera::intensity);
    METHODS(DeltaSigmaX,typename TypeHelper::ResolutionUnit);
    METHODS(DeltaSigmaY,typename TypeHelper::ResolutionUnit);
    METHODS(BestSigmaX,boost::units::camera::length);
    METHODS(BestSigmaY,boost::units::camera::length);
    METHODS(ZAtBestSigmaX,boost::units::si::nanolength);
    METHODS(ZAtBestSigmaY,boost::units::si::nanolength);
    METHODS(LayerDistance,boost::units::si::nanolength);
    METHODS(LayerShiftX,boost::units::camera::length);
    METHODS(LayerShiftY,boost::units::camera::length);
#undef METHODS
    typedef boost::units::quantity<boost::units::camera::intensity,double>
        QuantityShift;
    QuantityShift getShift() const 
        { return QuantityShift::from_value( get<Shift,0>() ); }
    void setShift(QuantityShift v) { return set<Shift,0>(v.value()); }

    template <int Function>
    Eigen::Vector3d getPosition() const {
        Eigen::Vector3d rv;
        rv.x() = double(getMeanX<Function>() / boost::units::camera::pixel);
        rv.y() = double(getMeanY<Function>() / boost::units::camera::pixel);
        rv.z() = double(getMeanZ<Function>() / boost::units::si::nanometre);
        return rv;
    }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

};

}
}

#endif
