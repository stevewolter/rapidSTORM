#ifndef NONLINFIT_IMAGEFUNCTION_H
#define NONLINFIT_IMAGEFUNCTION_H

#include <Eigen/StdVector>
#include <stdexcept>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/camera/intensity.hpp>
#include <boost/units/systems/si/area.hpp>
#include "Spot.h"

namespace dStorm {
namespace guf {

template <int Dimensions> struct Statistics;

template <>
struct Statistics<2> {
    Spot highest_pixel;
    boost::units::quantity<boost::units::camera::intensity> integral, peak_intensity, quarter_percentile_pixel;
    boost::units::quantity<boost::units::si::area> peak_pixel_area;
    int pixel_count;
};

template <>
struct Statistics<3> 
: private std::vector< Statistics<2> >,
  public Statistics<2>
{
    Spot highest_pixel;
    boost::units::quantity<boost::units::camera::intensity> integral, peak_intensity;
    int pixel_count;

    Statistics<3>() 
        : integral( 0 * boost::units::camera::ad_count ), 
          peak_intensity(integral),
          pixel_count(0) {}
    void push_back( const Statistics<2>& s ) {
        integral += s.integral;
        pixel_count += s.pixel_count;
        if ( s.peak_intensity > peak_intensity ) {
            peak_intensity = s.peak_intensity;
            peak_pixel_area = s.peak_pixel_area;
            highest_pixel = s.highest_pixel;
        }
        std::vector< Statistics<2> >::push_back(s);
    }
    const Statistics<2>& operator[](int i) const
        { return static_cast< const std::vector< Statistics<2> >& >(*this)[i]; }
    int size() const { return std::vector< Statistics<2> >::size(); }
};

}
}

#endif
