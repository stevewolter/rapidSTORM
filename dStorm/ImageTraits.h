#ifndef DSTORM_INPUT_IMAGETRAITS_H
#define DSTORM_INPUT_IMAGETRAITS_H

#include <Eigen/Core>
#include <dStorm/input/Traits.h>
#include "ImageTraits_decl.h"
#include <dStorm/SizeTraits.h>
#include <dStorm/Image_decl.h>
#include <dStorm/units_Eigen_traits.h>
#include <dStorm/units/frame_count.h>

namespace dStorm {
namespace input {

/** The Traits class partial specialization for images
 *  provides methods to determine the image dimensions and the
 *  resolution. */
template <typename PixelType, int Dimensions>
class Traits< dStorm::Image<PixelType,Dimensions> >
  : public SizeTraits<Dimensions>
{
  public:
    typedef typename SizeTraits<Dimensions>::Resolution Resolution;
    /** Dimensionality (number of colours) of image. */
    int dim;    

    Traits() : SizeTraits<Dimensions>(), dim(1) {}
    template <typename Type>
    Traits( const Traits< dStorm::Image<Type,Dimensions> >& o )
        : SizeTraits<Dimensions>(o), dim(o.dim) {}
    /** CImg compatibility method.
     *  @return Number of colors in image. */
    inline int dimv() const { return dim; }

    simparm::optional<frame_count> total_frame_count;
};

}
}

#endif
