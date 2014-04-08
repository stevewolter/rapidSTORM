#ifndef DSTORM_TRAITS_IMAGE_RESOLUTION_H
#define DSTORM_TRAITS_IMAGE_RESOLUTION_H

#include <string>
#include <boost/optional.hpp>
#include <boost/units/systems/camera/resolution.hpp>
#include <boost/units/systems/camera/length.hpp>
#include <boost/units/io.hpp>
#include <boost/units/systems/si/io.hpp>

namespace dStorm {
namespace traits {

struct ImageResolution { 
    std::string unit_name; std::string unit_symbol; 
  private:
    boost::optional< boost::units::quantity< boost::units::camera::resolution, float > > dpm_value;

  public:
    typedef boost::units::quantity< boost::units::camera::length, int> PixelCount;
    typedef boost::units::divide_typeof_helper< float, PixelCount >::type PerPixel;
    PerPixel value;

    ImageResolution() 
        : unit_name(""), unit_symbol(""), value( PerPixel::from_value(0)) {}
    template <typename Unit, typename Type>
    inline ImageResolution( const boost::units::quantity<Unit,Type>& o ); 

    bool is_in_dpm() const { return dpm_value.is_initialized(); }
    boost::units::quantity< boost::units::camera::resolution, float > in_dpm() const
        { return *dpm_value; }

    bool operator==( const ImageResolution& o ) const {
        return unit_name == o.unit_name && unit_symbol == o.unit_symbol 
            && dpm_value == o.dpm_value;
    }
};

template <typename Unit, typename Type>
ImageResolution::ImageResolution( const boost::units::quantity<Unit,Type>& o )
{
    typedef typename boost::units::multiply_typeof_helper<Unit,boost::units::camera::length>::type divided_unit;
    unit_name = boost::units::name_string(divided_unit());
    unit_symbol = boost::units::symbol_string(divided_unit());

    value = o / boost::units::quantity<divided_unit>::from_value(1);

    if ( unit_symbol == "nm" ) {
        boost::units::quantity< boost::units::si::length, float > l(1E-9 * boost::units::si::metre);
        dpm_value = 1.0f / (l * value);
    } else if ( unit_symbol == "m" ) {
        dpm_value = 1.0f / (static_cast< boost::units::quantity< boost::units::si::length,float > >(1.0 * boost::units::si::metre) * value);
    }
}



}
}

#endif
