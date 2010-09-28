#ifndef DSTORM_INPUT_IMAGETRAITS_H
#define DSTORM_INPUT_IMAGETRAITS_H

#include <Eigen/Core>
#include <dStorm/input/Traits.h>
#include "ImageTraits_decl.h"
#include <dStorm/SizeTraits.h>
#include <dStorm/Image_decl.h>
#include <dStorm/units_Eigen_traits.h>
#include <dStorm/units/frame_count.h>
#include <dStorm/units/frame_rate.h>
#include <dStorm/units/camera_response.h>

namespace dStorm {
namespace input {

struct GenericImageTraits
{
    /** Dimensionality (number of colours) of image. */
    int dim;    

    frame_count first_frame;
    simparm::optional<frame_count> last_frame;
    simparm::optional<frame_rate> speed;
    simparm::optional<camera_response> photon_response;

    bool background_standard_deviation_is_set;

    GenericImageTraits();
};

/** The Traits class partial specialization for images
 *  provides methods to determine the image dimensions and the
 *  resolution. */
template <typename PixelType, int Dimensions>
class Traits< dStorm::Image<PixelType,Dimensions> >
  : public SizeTraits<Dimensions>,
    public GenericImageTraits
{
  public:
    typedef typename SizeTraits<Dimensions>::Resolution Resolution;

    Traits();
    template <typename Type>
    Traits( const Traits< dStorm::Image<Type,Dimensions> >& o );
    /** CImg compatibility method.
     *  @return Number of colors in image. */
    inline int dimv() const { return GenericImageTraits::dim; }
};

}
}

#endif
