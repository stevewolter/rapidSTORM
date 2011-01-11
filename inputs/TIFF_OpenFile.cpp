#undef NDEBUG
#define DSTORM_TIFFLOADER_CPP
#include <tiffio.h>
#include "TIFF.h"
#include "TIFFOperation.h"
#include <dStorm/ImageTraits.h>
#include <dStorm/ImageTraits_impl.h>
#include <boost/units/io.hpp>

namespace dStorm {
namespace TIFF {

static ttag_t resolution_tags[2] = { TIFFTAG_XRESOLUTION, TIFFTAG_YRESOLUTION };

OpenFile::OpenFile(const std::string& filename, const Config& config, simparm::Node& n)
: ignore_warnings(config.ignore_warnings()),
  file_ident(filename),
  current_directory(0),
  _width(0), _height(0), _no_images(-1)
{
    TIFFOperation op( "in opening TIFF file",
                      n, ignore_warnings );
    tiff = TIFFOpen( filename.c_str(), "r" );
    if ( tiff == NULL ) { op.throw_exception_for_errors(); throw std::logic_error("Undefined error in TIFF reading"); }

    uint32 width, height;
    TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &width );
    TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &height );
    _width = width; _height = height;

    for (int i = 0; i < 2; ++i) {
        float res;
        int given = TIFFGetField( tiff, resolution_tags[i], &res );
        if ( given == 1 ) {
            int unit = RESUNIT_INCH;
            TIFFGetField( tiff, TIFFTAG_RESOLUTIONUNIT, &unit );
            simparm::optional< boost::units::quantity<camera::resolution,float> > r;
            if ( unit == RESUNIT_INCH )
                r = res * camera::pixel / (0.0254f * boost::units::si::meters);
            else if ( unit == RESUNIT_CENTIMETER )
                r = res * camera::pixel / (0.0254f * boost::units::si::meters);

            if ( r.is_set() )
                resolution[i] = ImageResolution(1.0f / *r);
        }
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
    rv->size.x() = _width * camera::pixel;
    rv->size.y() = _height * camera::pixel;
    rv->dim = 1; /* TODO: Read from file */
    rv->resolution = resolution;
    if ( _no_images != -1 )
        rv->image_number().range().second = (_no_images - 1) * camera::frame;

    return rv;
}

template std::auto_ptr< Traits<dStorm::Image<unsigned short,2> > > 
OpenFile::getTraits();

OpenFile::~OpenFile() {
    TIFFClose( tiff );
}

}
}
