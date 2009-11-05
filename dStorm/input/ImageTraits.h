#ifndef CIMGBUFFER_IMAGETRAITS_H
#define CIMGBUFFER_IMAGETRAITS_H

#include <dStorm/input/Traits.h>
#include <Eigen/Core>

namespace cimg_library {
    template <typename PixelType> class CImg;
};

namespace CImgBuffer {

class Config;

/** The Traits class partial specialization for images
 *  provides methods to determine the image dimensions and the
 *  resolution. */
template <typename PixelType>
class Traits< cimg_library::CImg<PixelType> > {
  public:
    /** Vector giving the size of the image in pixels in the three
     *  spatial dimensions. */
    Eigen::Matrix<int,3,1,Eigen::DontAlign> size;
    /** Dimensionality (number of colours) of image. */
    int dim;    
    /** Resolution in the three spatial dimensions given in meters per
     *  pixel. */
    Eigen::Matrix<double,3,1,Eigen::DontAlign> resolution;

    Traits() : size(0,0,1), dim(1), resolution(0,0,0) {}
    template <typename Type>
    Traits( const Traits< cimg_library::CImg<Type> >& o )
        : size(o.size), dim(o.dim), resolution(o.resolution) {}
    /** CImg compatibility method.
     *  @return X component of \c size, i.e. width of image in pixels. */
    inline int dimx() const { return size.x(); }
    /** CImg compatibility method.
     *  @return Y component of \c size, i.e. height of image in pixels. */
    inline int dimy() const { return size.y(); }
    /** CImg compatibility method.
     *  @return Z component of \c size, i.e. depth of image in pixels. */
    inline int dimz() const { return size.z(); }
    /** CImg compatibility method.
     *  @return Number of colors in image. */
    inline int dimv() const { return dim; }

    void apply_global_settings(const Config& c) { compute_resolution(c); }
    void compute_resolution( const Config& );
    void set_resolution( const Eigen::Vector3d& resolution );
};
};

#endif
