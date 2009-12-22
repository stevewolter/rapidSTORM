#ifndef DSTORM_INPUT_IMAGETRAITS_H
#define DSTORM_INPUT_IMAGETRAITS_H

#include <Eigen/Core>
#include "Traits.h"
#include "Config_decl.h"
#include "ImageTraits_decl.h"
#include <dStorm/SizeTraits.h>

namespace dStorm {
namespace input {

/** The Traits class partial specialization for images
 *  provides methods to determine the image dimensions and the
 *  resolution. */
template <typename PixelType>
class Traits< cimg_library::CImg<PixelType> >
  : public SizeTraits<3>
{
  public:
    /** Dimensionality (number of colours) of image. */
    int dim;    

    Traits() : SizeTraits<3>(), dim(1) {}
    template <typename Type>
    Traits( const Traits< cimg_library::CImg<Type> >& o )
        : SizeTraits<3>(o), dim(o.dim) {}
    /** CImg compatibility method.
     *  @return Number of colors in image. */
    inline int dimv() const { return dim; }

    void apply_global_settings(const Config& c) { compute_resolution(c); }
    void compute_resolution( const Config& );
    void set_resolution( const Eigen::Vector3d& resolution );
};

}
}

#endif
