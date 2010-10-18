#define DSTORM_TIFFLOADER_CPP
#include <tiffio.h>
#include "TIFF.h"
#include "TIFFOperation.h"
#include <dStorm/ImageTraits.h>

namespace dStorm {
namespace TIFF {

OpenFile::OpenFile(const std::string& filename, const Config& config, simparm::Node& n)
: ignore_warnings(config.ignore_warnings()),
  file_ident(filename),
  current_directory(0),
  _width(0), _height(0), _no_images(-1)
{
    TIFFOperation op( "in opening TIFF file",
                      n, ignore_warnings );
    tiff = TIFFOpen( filename.c_str(), "rm" );
    if ( tiff == NULL ) op.throw_exception_for_errors();

    TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &_width );
    TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &_height );

    float xres, yres;
    int xgiven = TIFFGetField( tiff, TIFFTAG_XRESOLUTION, &xres );
    int ygiven = TIFFGetField( tiff, TIFFTAG_YRESOLUTION, &yres );
    if ( xgiven == 1 || ygiven == 1 ) {
        float res;
        if ( xgiven == 1 && ygiven == 1 )
            res = (xres+yres)/2;
        else if ( xgiven == 1 )
            res = xres;
        else
            res = yres;

        int unit = RESUNIT_INCH;
        TIFFGetField( tiff, TIFFTAG_RESOLUTIONUNIT, &unit );
        if ( unit == RESUNIT_INCH )
            resolution = boost::units::quantity<cs_units::camera::resolution,float>(
                res * cs_units::camera::pixel / (0.0254 * boost::units::si::meters));
        else if ( unit == RESUNIT_CENTIMETER )
            resolution = boost::units::quantity<cs_units::camera::resolution,float>(
                res * cs_units::camera::pixel / (0.01 * boost::units::si::meters));
    }

    if ( config.determine_length() ) {
        _no_images = 1;
        while ( TIFFReadDirectory(tiff) != 0 )
            _no_images += 1;
        TIFFSetDirectory(tiff, 0);
    }
}

template<typename Pixel>                                   
typename std::auto_ptr< Traits<dStorm::Image<Pixel,2> > > 
OpenFile::getTraits() 
{
    std::auto_ptr< Traits<dStorm::Image<Pixel,2> > >
        rv( new Traits<dStorm::Image<Pixel,2> >() );
    rv->size.x() = _width * cs_units::camera::pixel;
    rv->size.y() = _height * cs_units::camera::pixel;
    rv->dim = 1; /* TODO: Read from file */
    rv->resolution = resolution;
    if ( _no_images != -1 )
        rv->last_frame = (_no_images - 1) * cs_units::camera::frame;

    return rv;
}

template std::auto_ptr< Traits<dStorm::Image<unsigned short,2> > > 
OpenFile::getTraits();

OpenFile::~OpenFile() {
    TIFFClose( tiff );
}

}
}
