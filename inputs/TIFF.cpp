#define DSTORM_TIFFLOADER_CPP

#include <stdexcept>
#include <cassert>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <stdint.h>

#include <simparm/ChoiceEntry_Impl.hh>

#include <CImg.h>
#include <tiffio.h>

#include "TIFF.h"
#include <dStorm/input/Source.h>
#include <dStorm/input/Source_impl.h>
#include <dStorm/input/ImageTraits.h>
#include <dStorm/input/FileBasedMethod_impl.h>

using namespace std;
using namespace cimg_library;

namespace dStorm {
namespace TIFF {

static char tiff_error_buffer[4096];

template<typename Pixel>
void Source<Pixel>::TIFF_error_handler(
    const char* module, const char *fmt, va_list ap)
{
    vsnprintf( tiff_error_buffer, 4096, fmt, ap );
}

template<typename Pixel>
Source<Pixel>::Source(const char *src)
: simparm::Set("TIFF", "TIFF image reader"),
  SerialSource< CImg<Pixel> >
    ( static_cast<simparm::Node&>(*this),    
      BaseSource::Pushing | BaseSource::Pullable)
{
    TIFFSetErrorHandler( TIFF_error_handler );
    tiff = TIFFOpen( src, "rm" );
    if ( tiff == NULL ) throw_error();

    Traits< CImg<Pixel> >& my_traits = *this;
    TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &_width );
    TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &_height );
    my_traits.size.x() = _width * cs_units::camera::pixel;
    my_traits.size.y() = _height * cs_units::camera::pixel;
    my_traits.size.z() = 1 * cs_units::camera::pixel; 
            /* TODO: Read from file */
    my_traits.dim = 1; /* TODO: Read from file */

    _no_images = 1;
    while ( TIFFReadDirectory(tiff) != 0 )
        _no_images += 1;

    TIFFSetDirectory(tiff, 0);
}

template<typename Pixel>
void Source<Pixel>::throw_error()
{
    throw std::logic_error( tiff_error_buffer );
}

template<typename Pixel>
CImg<Pixel>*
Source<Pixel>::load()
{
    uint32_t width, height;
    uint16_t bitspersample;
    TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &width );
    TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height );
    TIFFGetField( tiff, TIFFTAG_BITSPERSAMPLE, &bitspersample );

    if ( int(width) != _width || int(height) != _height ) {
        std::stringstream error;
        error << "TIFF image no. " << TIFFCurrentDirectory(tiff)
              << " has dimensions (" << width << ", " << height
              << ") different from first image (" << _width
              << ", " << _height << "). Aborting.";
        throw std::runtime_error( error.str() );
    }
    if ( bitspersample != sizeof(Pixel)*8 ) {
        std::stringstream error;
        error << "TIFF image no. " << TIFFCurrentDirectory(tiff) << " has "
              << bitspersample << " bits per pixel, but "
              << sizeof(Pixel)*8 << " bits are necessary.";
        throw std::runtime_error( error.str() );
    }

    tsize_t strip_size = TIFFStripSize( tiff );
    tstrip_t strip_count = TIFFNumberOfStrips( tiff );
    std::auto_ptr< CImg<Pixel> > img( new CImg<Pixel>(width, height) );

    assert( img->size() * sizeof(Pixel) >= strip_size * strip_count );

    for (tstrip_t strip = 0; strip < strip_count; strip++) {
        TIFFReadEncodedStrip( tiff, strip, 
            img->ptr() + (strip * strip_size / sizeof(Pixel)),
            strip_size );
    }

    TIFFReadDirectory(tiff);
    return img.release();
}

template<typename Pixel>
Source<Pixel>::~Source() {
    TIFFClose( tiff );
}

template<typename Pixel>
Source< Pixel >*
Config<Pixel>::impl_makeSource()
{
    Source<Pixel>* ptr =
        new Source<Pixel>(this->inputFile().c_str());
    ptr->push_back( this->inputFile );
    this->inputFile.editable = false;
    return ptr;
}

template<typename Pixel>
Config<Pixel>::Config( input::Config& src) 
: FileBasedMethod< CImg<Pixel> >(
        src, "TIFF", "TIFF file", 
        "extension_tif", ".tif" ),
  tiff_extension("extension_tiff", ".tiff")
{
    this->inputFile.push_back(tiff_extension);
    this->push_back( src.pixel_size_in_nm );
}

template<typename Pixel>
Config<Pixel>::Config(
    const Config<Pixel>::Config &c,
    input::Config& src
) 
: FileBasedMethod< CImg<Pixel> >(c, src),
  tiff_extension(c.tiff_extension)
{
    this->inputFile.push_back(tiff_extension);
    this->push_back( src.pixel_size_in_nm );
}

template class Config<unsigned char>;
template class Config<unsigned short>;
template class Config<unsigned int>;
template class Config<float>;
template class Config<double>;

}
}
