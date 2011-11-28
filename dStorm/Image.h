#ifndef DSTORM_IMAGE_H
#define DSTORM_IMAGE_H

#include "Image_decl.h"
#include "units/frame_count.h"
#include <boost/shared_array.hpp>
#include <boost/units/pow.hpp>
#include <boost/units/systems/camera/length.hpp>
#include <boost/units/systems/camera/area.hpp>
#include "units/camera_response.h"
#include <Eigen/Core>
#include <boost/units/Eigen/Core>
#include "units/camera_response.h"
#include <boost/static_assert.hpp>

namespace dStorm {

template <int Dimensions>
struct ImageTypes {
    typedef boost::units::quantity<camera::length,int> Span;
    typedef Eigen::Matrix<Span,Dimensions,1,Eigen::DontAlign> Size;
    typedef Eigen::Matrix<int,Dimensions,1,Eigen::DontAlign> Position;
    typedef Eigen::Matrix<int,Dimensions,1,Eigen::DontAlign> Offsets;
};

template <typename PixelType, int Dimensions>
class Image
: public ImageTypes<Dimensions>
{
  public:
    typedef PixelType Pixel;
    static const int Dim = Dimensions;
    typedef std::pair<PixelType,PixelType> PixelPair;
    typedef typename ImageTypes<Dimensions>::Span Span;
    typedef typename ImageTypes<Dimensions>::Size Size;
    typedef typename ImageTypes<Dimensions>::Position Position;
    typedef typename ImageTypes<Dimensions>::Offsets Offsets;

  protected:
    template <typename OtherPixelType, int OtherDimension> friend class Image;
    boost::shared_array<PixelType> img;
    Size sz;
    Offsets offsets;
    size_t global_offset;
    frame_index fn;
    camera_response bg_sigma;

    template <typename IteratedType>
    class _iterator;
    
  public:
    Image();
    Image(Size sz, frame_index i = 0 * camera::frame);
    Image(Size sz, boost::shared_array<PixelType> data, Offsets offsets, size_t global_offset, frame_index i);
    ~Image();

    void invalidate() { img.reset(); }
    bool is_invalid() const { return img.get() == NULL; }
    bool is_valid() const { return img.get() != NULL; }

    boost::shared_array<PixelType> get_data_reference() const { return img; }

    void fill(PixelType type);
    PixelPair minmax() const;
    template <typename ReducedPixel>
        Image<ReducedPixel,Dimensions> normalize() const;
    template <typename ReducedPixel>
        Image<ReducedPixel,Dimensions> convert() const;
    template <typename ReducedPixel>
        Image<ReducedPixel,Dimensions> 
        normalize( const PixelPair& minmax ) const;
    Image<bool,Dimensions> threshold( PixelType threshold ) const;
    Image deep_copy() const;

    const PixelType& operator()(int x, int y) const { 
        BOOST_STATIC_ASSERT(Dimensions == 2);
        return ptr()[x*offsets.x()+y*offsets.y()]; 
    }
    const PixelType& operator()(Span x, Span y) const { 
        BOOST_STATIC_ASSERT(Dimensions == 2);
        return ptr()[x.value()*offsets.x()+y.value()*offsets.y()]; 
    }
    PixelType& operator()(int x, int y) { 
        BOOST_STATIC_ASSERT(Dimensions == 2);
        return ptr()[x*offsets.x()+y*offsets.y()]; 
    }
    PixelType& operator()(Span x, Span y) { 
        BOOST_STATIC_ASSERT(Dimensions == 2);
        return ptr()[x.value()*offsets.x()+y.value()*offsets.y()]; 
    }
    const PixelType& operator()(int x, int y, int z) const { 
        BOOST_STATIC_ASSERT(Dimensions == 3);
        return ptr()[x*offsets.x()+y*offsets.y()+z*offsets.z()]; 
    }
    PixelType& operator()(int x, int y, int z) { 
        BOOST_STATIC_ASSERT(Dimensions == 3);
        return ptr()[x*offsets.x()+y*offsets.y()+z*offsets.z()]; 
    }

    const PixelType *ptr() const { return img.get() + global_offset; }
    PixelType *ptr() { return img.get() + global_offset; }
    const PixelType *ptr(int x, int y) const { 
        BOOST_STATIC_ASSERT(Dimensions == 2);
        return &(*this)(x,y); 
    }
    PixelType *ptr(int x, int y) { 
        BOOST_STATIC_ASSERT(Dimensions == 2);
        return &(*this)(x,y); 
    }

    Span width() const { return sz.x(); }
    int width_in_pixels() const 
        { return sz.x() / camera::pixel; }
    Span height() const { return sz.y(); }
    int height_in_pixels() const { 
        BOOST_STATIC_ASSERT(Dimensions >= 2);
        return sz.y() / camera::pixel; }
    Span depth() const { return sz.z(); }
    int depth_in_pixels() const { 
        BOOST_STATIC_ASSERT(Dimensions >= 3);
        return sz.z() / camera::pixel; }
    const Size& sizes() const { return sz; }

    const Offsets& get_offsets() const { return offsets; }
    size_t get_global_offset() const { return global_offset; }

    typedef typename boost::units::power_typeof_helper<
            camera::length,
            boost::units::static_rational<Dimensions>
        >::type AreaUnit;
    typedef boost::units::quantity<AreaUnit,unsigned long>
        AreaQuantity;
    AreaQuantity size() const 
        { return AreaQuantity::from_value(size_in_pixels()); }
    unsigned long size_in_pixels()  const { 
        unsigned long rv = 1;
        for (int i = 0; i < Dimensions; ++i) rv *= sz[i].value();
        return rv;
    }
    frame_index frame_number() const { return fn; }
    frame_index& frame_number() { return fn; }

    camera_response background_standard_deviation() const { return bg_sigma; }
    camera_response& background_standard_deviation() { return bg_sigma; }

    typedef _iterator<PixelType> iterator;
    typedef _iterator<const PixelType> const_iterator;

    const_iterator begin() const;
    iterator begin();
    const_iterator end() const;
    iterator end();

    PixelType& operator[](const int i) 
        { return this->ptr()[i]; }
    const PixelType& operator[](const int i) const
        { return this->ptr()[i]; }

    bool contains( int x, int y ) { 
        BOOST_STATIC_ASSERT( Dimensions == 2 );
        return ( x >= 0 && x < this->width_in_pixels() &&
                   y >= 0 && y < this->height_in_pixels() ); 
    }

    inline Image<PixelType,Dimensions-1> slice( int dimension, typename Size::Scalar layer ) const;
};

}
#endif
