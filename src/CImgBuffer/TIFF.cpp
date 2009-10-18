#define CImgBuffer_TIFFLOADER_CPP

#include <stdexcept>
#include <cassert>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <functional>

#include <simparm/ChoiceEntry_Impl.hh>

#include <CImg.h>
#include <tiffio.h>

#include "TIFF.h"
#include <CImgBuffer/Source.h>
#include <CImgBuffer/Source_impl.h>
#include <CImgBuffer/ImageTraits.h>

using namespace std;
using namespace cimg_library;

namespace CImgBuffer {
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
: SerialImageSource<Pixel>
    (BaseSource::Pushing | BaseSource::Pullable),
   simparm::Set("TIFF", "TIFF image reader"),
   _width( this->Traits< CImg<Pixel> >::size.x() ),
   _height( this->Traits< CImg<Pixel> >::size.y() )
{
    TIFFSetErrorHandler( TIFF_error_handler );
    tiff = TIFFOpen( src, "r" );
    if ( tiff == NULL ) throw_error();

    Traits< CImg<Pixel> >& my_traits = *this;
    TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &_width );
    TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &_height );
    my_traits.size.z() = 1;     /* TODO: Read from file */
    my_traits.dim = 1;          /* TODO: Read from file */

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

    assert( img->size() >= sizeof(Pixel) * strip_size * strip_count );

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
        new Source<Pixel>(inputFile().c_str());
    ptr->push_back( inputFile );
    inputFile.editable = false;
    return ptr;
}

template<typename Pixel>
Config<Pixel>::Config( CImgBuffer::Config& src) 
: InputConfig< CImg<Pixel> >("TIFF", "TIFF file"),
  master(src),
  inputFile(src.inputFile),
  tiff_extension("extension_tiff", ".tiff"),
  tif_extension("extension_tif", ".tif")
{
    this->register_entry(&inputFile);
    inputFile.push_back(tif_extension);
    inputFile.push_back(tiff_extension);
}

template<typename Pixel>
Config<Pixel>::Config(
    const Config<Pixel>::Config &c,
    CImgBuffer::Config& src
) 
: simparm::Node(c),
  InputConfig< CImg<Pixel> >(c),
  master(src),
  inputFile(src.inputFile),
  tiff_extension(c.tiff_extension),
  tif_extension(c.tif_extension)
{
    this->register_entry(&inputFile);
    inputFile.push_back(tif_extension);
    inputFile.push_back(tiff_extension);
}

template class Config<unsigned char>;
template class Config<unsigned short>;
template class Config<unsigned int>;
template class Config<float>;
template class Config<double>;

}
}
