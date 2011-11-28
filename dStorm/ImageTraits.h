#ifndef DSTORM_INPUT_IMAGETRAITS_H
#define DSTORM_INPUT_IMAGETRAITS_H

#include <boost/units/systems/camera/length.hpp>
#include <boost/units/systems/camera/resolution.hpp>
#include <boost/units/quantity.hpp>
#include <Eigen/Core>
#include "input/Traits.h"
#include "ImageTraits_decl.h"
#include "Image_decl.h"
#include <boost/units/Eigen/Core>
#include "units/camera_response.h"
#include "DataSetTraits.h"
#include "traits/image_number.h"
#include <boost/optional.hpp>
#include "Image.h"
#include "traits/optics.h"

namespace dStorm {
namespace input {

struct GenericImageTraits 
{
    /** Dimensionality (number of colours) of image. */
    int dim;    

    GenericImageTraits();
};

template <int Dimensions>
struct ImageTraits 
: public GenericImageTraits
{
  public:
    typedef Eigen::Matrix< 
            boost::units::quantity<camera::length,int>,
            Dimensions,1,Eigen::DontAlign> 
        Size;
    Size size;

    ImageTraits();
    template <int OtherDimensions>
    inline ImageTraits( const ImageTraits<OtherDimensions>& other );

    /** CImg compatibility method.
     *  @return Number of colors in image. */
    inline int dimv() const { return GenericImageTraits::dim; }
    std::string desc() const { return "image"; }

    ImageTraits* clone() const { return new ImageTraits(*this); }
};

/** The Traits class partial specialization for images
 *  provides methods to determine the image dimensions and the
 *  resolution. */
template <typename PixelType, int Dimensions>
class Traits< dStorm::Image<PixelType,Dimensions> >
: public input::BaseTraits, public DataSetTraits,
  public ImageTraits<Dimensions>,
  public traits::Optics<Dimensions>
{
  public:
    typedef Eigen::Matrix< boost::units::quantity< boost::units::si::length, float >, 2,1, Eigen::DontAlign > PSF;
  private:
    boost::optional< PSF > psf;
    traits::ImageNumber in;
  public:
    Traits() {}
    template <typename Type>
    inline Traits( const Traits< dStorm::Image<Type,Dimensions> >& o );
    Traits* clone() const { return new Traits(*this); }
    std::string desc() const { return "image"; }

    traits::ImageNumber& image_number() { return in; }
    const traits::ImageNumber& image_number() const { return in; }

    boost::optional<PSF>& psf_size() { return psf; }
    const boost::optional<PSF>& psf_size() const { return psf; }

};

#if 0
template <int Dim>
template <int OtherDim>
ImageTraits<Dim>::ImageTraits( const ImageTraits<OtherDim>& other )
: GenericImageTraits(other), traits::ImageNumber(other)
{
    if ( OtherDim > Dim )
        size = other.size.template start<Dim>();
    else {
        size.fill( 1 * boost::units::camera::pixel );
        size.template start<OtherDim>() = other.size;
    }
}
#endif

}
}

#endif
