#ifndef DSTORM_BASEIMAGE_H
#define DSTORM_BASEIMAGE_H

#include "BaseImage_decl.h"
#include "units/frame_count.h"
#include <dStorm/SizeTraits.h>
#include <boost/shared_array.hpp>
#include <boost/units/pow.hpp>
#include <cs_units/camera/area.hpp>

namespace dStorm {

template <typename PixelType, int Dimensions>
class BaseImage {
  public:
    typedef boost::units::quantity<cs_units::camera::length,int> Span;
    typedef Eigen::Matrix<Span,Dimensions,1,Eigen::DontAlign> Size;
  protected:
    boost::shared_array<PixelType> img;
    Size sz;
    frame_index fn;
    unsigned long _pxc;

    template <typename IteratedType>
    class _iterator;
    
  public:
    BaseImage();
    BaseImage(Size sz, frame_index i);
    BaseImage(Size sz, boost::shared_array<PixelType> data,
              frame_index i);

    void invalidate() { img.reset(); }
    bool is_invalid() const { return img.get() == NULL; }

    const PixelType& operator()(int x, int y) const
        { return img[x+y*sz.x().value()]; }
    PixelType& operator()(int x, int y) 
        { return img[x+y*sz.x().value()]; }
    const PixelType *ptr() const { return img.get(); }
    PixelType *ptr() { return img.get(); }
    const PixelType *ptr(int x, int y) const
        { return &(*this)(x,y); }
    PixelType *ptr(int x, int y) 
        { return &(*this)(x,y); }

    Span width() const { return sz.x(); }
    int width_in_pixels() const 
        { return sz.x() / cs_units::camera::pixel; }
    Span height() const { return sz.y(); }
    int height_in_pixels() const 
        { return sz.y() / cs_units::camera::pixel; }
    const Size& sizes() const { return sz; }

    typedef typename boost::units::power_typeof_helper<
            cs_units::camera::length,
            boost::units::static_rational<Dimensions>
        >::type AreaUnit;
    typedef boost::units::quantity<AreaUnit,unsigned long>
        AreaQuantity;
    AreaQuantity size() const 
        { return AreaQuantity::from_value(_pxc); }
    unsigned long size_in_pixels()  const
        { return _pxc; }
    frame_index frame_number() const { return fn; }
    frame_index& frame_number() { return fn; }


    typedef _iterator<PixelType> iterator;
    typedef _iterator<const PixelType> const_iterator;

    const_iterator begin() const;
    iterator begin();
    const_iterator end() const;
    iterator end();

    PixelType& operator[](const int i) 
        { return this->img[i]; }
    const PixelType& operator[](const int i) const
        { return this->img[i]; }
};

}

#endif
