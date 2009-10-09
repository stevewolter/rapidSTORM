#ifndef CIMGBUFFER_IMAGETRAITS_H
#define CIMGBUFFER_IMAGETRAITS_H

#include <CImgBuffer/Traits.h>
#include <Eigen/Core>

namespace cimg_library {
    template <typename PixelType> class CImg;
};

namespace CImgBuffer {

class Config;

/** The Traits class partial specialization for images
 *  provides methods to determine the image coordinates. */
template <typename PixelType>
class Traits< cimg_library::CImg<PixelType> > {
    double xr, yr, zr;
  public:
    Traits() : xr(0), yr(0), zr(0) {}
    virtual int dimx() const = 0;
    virtual int dimy() const = 0;
    virtual int dimz() const { return 1; }
    virtual int dimv() const { return 1; }

    void apply_global_settings(const Config& c) { compute_resolution(c); }
    void compute_resolution( const Config& );
    virtual double x_resolution() const { return xr; }
    virtual double y_resolution() const { return yr; }
    virtual double z_resolution() const { return zr; }
    Eigen::Vector3d resolution() const 
        { return Eigen::Vector3d(xr, yr, zr); }
};
};

#endif
