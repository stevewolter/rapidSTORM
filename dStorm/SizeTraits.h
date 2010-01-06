#ifndef DSTORM_SIZETRAITS_H
#define DSTORM_SIZETRAITS_H

#include <Eigen/Core>
#include <dStorm/units.h>
#include "units_Eigen_traits.h"

namespace dStorm {

template <int Dimensions>
struct SizeTraits {
    typedef Eigen::Matrix< 
            quantity<camera::length,int>,
                Dimensions,1,Eigen::DontAlign> 
        Size;
    typedef Eigen::Matrix<
            quantity<camera::pixel_size,float>,
            Dimensions,1,Eigen::DontAlign> 
        Resolution;

    Size size;
    Resolution resolution;

    SizeTraits() : size( Size::Zero() ),
                   resolution( Resolution::Zero() ) {}

    /** CImg compatibility method.
     *  @return X component of \c size, i.e. width of image in pixels. */
    inline pixel_count dimx() const { return size.x(); }
    /** CImg compatibility method.
     *  @return Y component of \c size, i.e. height of image in pixels. */
    inline pixel_count dimy() const { return size.y(); }
    /** CImg compatibility method.
     *  @return Z component of \c size, i.e. depth of image in pixels. */
    inline pixel_count dimz() const 
        { return (Dimensions >= 3) ?  size.z()
                                   : 1*camera::pixel; }

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
        rv.size[i] = 0 * camera::pixel;
        rv.resolution[i] = 0 * si::meter / camera::pixel;
    }
    
    return rv;
}

}

#endif
