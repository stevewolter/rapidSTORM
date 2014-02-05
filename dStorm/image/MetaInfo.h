#ifndef DSTORM_INPUT_IMAGETRAITS_H
#define DSTORM_INPUT_IMAGETRAITS_H

#include <Eigen/Core>
#include <boost/units/systems/camera/length.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/Eigen/Core>
#include "dStorm/image/fwd.h"
#include <boost/array.hpp>
#include <dStorm/traits/image_resolution.h>

namespace dStorm {
namespace image {

template <int Dimensions>
class MetaInfo 
{
public:
    typedef boost::array< boost::optional<traits::ImageResolution>,Dimensions> Resolutions;
    typedef Eigen::Matrix< 
            boost::units::quantity<boost::units::camera::length,int>,
            Dimensions,1,Eigen::DontAlign> 
        Size;

    Size size;

    MetaInfo();

    traits::ImageResolution resolution(int r) const;
    bool has_resolution(int dim) const;
    const Resolutions& image_resolutions() const;
    void set_resolution( int dimension, const traits::ImageResolution& resolution );
    void set_resolution( const Resolutions& f );

    const Size upper_bound() const { return size.array() - 1 * boost::units::camera::pixel; }
private:
    Resolutions resolutions_;
};

}
}

#endif
