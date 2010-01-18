#ifndef DSTORM_SIZETRAITS_H
#define DSTORM_SIZETRAITS_H

#include <Eigen/Core>
#include <boost/units/quantity.hpp>
#include <cs_units/camera/length.hpp>
#include <cs_units/camera/resolution.hpp>
#include "units_Eigen_traits.h"
#include <limits>

namespace dStorm {

template <int Dimensions>
struct SizeTraits {
    typedef Eigen::Matrix< 
            boost::units::quantity<cs_units::camera::length,int>,
            Dimensions,1,Eigen::DontAlign> 
        Size;
    typedef Eigen::Matrix<
            boost::units::quantity<cs_units::camera::resolution,float>,
            Dimensions,1,Eigen::DontAlign> 
        Resolution;

    Size size;
    Resolution resolution;

    SizeTraits() : size( Size::Zero() ),
                   resolution( Resolution::Zero() ) {}

    /** CImg compatibility method.
     *  @return X component of \c size, i.e. width of image in pixels. */
    inline typename Size::Scalar dimx() const { return size.x(); }
    /** CImg compatibility method.
     *  @return Y component of \c size, i.e. height of image in pixels. */
    inline typename Size::Scalar dimy() const { return size.y(); }
    /** CImg compatibility method.
     *  @return Z component of \c size, i.e. depth of image in pixels. */
    inline typename Size::Scalar dimz() const 
        { return (Dimensions >= 3) ?  size.z()
                                   : (1*cs_units::camera::pixel); }

    template <int NewDimensions>
    SizeTraits<NewDimensions> get_other_dimensionality() const; 
};

template <int Dimensions>
template <int NewDimensions>
SizeTraits<NewDimensions> 
SizeTraits<Dimensions>::get_other_dimensionality() const
{
    static const int CommonDimensions = 
        (Dimensions > NewDimensions) ? NewDimensions : Dimensions;

    SizeTraits<NewDimensions> rv;
    rv.size.start(CommonDimensions) = size.start(CommonDimensions);
    rv.resolution.start(CommonDimensions) = 
        resolution.start(CommonDimensions);

    for (int i = CommonDimensions; i < NewDimensions; i++) {
        rv.size[i] = Size::Scalar::from_value(
            std::numeric_limits<float>::signaling_NaN() );
        rv.resolution[i] = Resolution::Scalar::from_value(
            std::numeric_limits<float>::signaling_NaN() );
    }
    
    return rv;
}

}

#endif
