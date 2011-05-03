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
static ttag_t size_tags[3] = { TIFFTAG_IMAGEWIDTH, TIFFTAG_IMAGELENGTH, TIFFTAG_IMAGEDEPTH };

OpenFile::OpenFile(const std::string& filename, const Config& config, simparm::Node& n)
: ignore_warnings(config.ignore_warnings()),
  file_ident(filename),
  current_directory(0),
  _no_images(-1)
{
    size[0] = size[1] = size[2] = 0;
    TIFFOperation op( "in opening TIFF file",
                      n, ignore_warnings );
    tiff = TIFFOpen( filename.c_str(), "rCm" );
    if ( tiff == NULL ) { op.throw_exception_for_errors(); throw std::logic_error("Undefined error in TIFF reading"); }

    for (int i = 0; i < 3; ++i ) {
        uint32_t field;
        if ( ! TIFFGetField( tiff, size_tags[i], &field ) )
            field = 1;
        size[i] = field;
    }

    int unit = RESUNIT_INCH;
    TIFFGetField( tiff, TIFFTAG_RESOLUTIONUNIT, &unit );
    for (int i = 0; i < 2; ++i) {
        float res;
        int given = TIFFGetField( tiff, resolution_tags[i], &res );
        if ( given == 1 ) {
            simparm::optional< boost::units::quantity<camera::resolution,float> > r;
            if ( unit == RESUNIT_INCH )
                r = res * camera::pixel / (0.0254f * boost::units::si::meters);
            else if ( unit == RESUNIT_CENTIMETER )
                r = res * camera::pixel / (0.01f * boost::units::si::meters);

            if ( r.is_set() )
                resolution[i] = traits::ImageResolution(1.0f / *r);
        }
    }

    if ( config.determine_length() ) {
        _no_images = 1;
        while ( TIFFReadDirectory(tiff) != 0 )
            _no_images += 1;
        TIFFSetDirectory(tiff, 0);
    }
}

template<typename Pixel, int Dim>                                   
typename std::auto_ptr< Traits<dStorm::Image<Pixel,Dim> > > 
OpenFile::getTraits() 
{
    BOOST_STATIC_ASSERT( Dim <= 3 );
    std::auto_ptr< Traits<dStorm::Image<Pixel,Dim> > >
        rv( new Traits<dStorm::Image<Pixel,Dim> >() );
    for (int i = 0; i < Dim; ++i) rv->size[i] = size[i] * camera::pixel;
    rv->dim = 1; /* TODO: Read from file */
    for (int p = 0; p < rv->plane_count(); ++p)
        rv->plane(p).set_resolution( resolution );
    if ( _no_images != -1 )
        rv->image_number().range().second = (_no_images - 1) * camera::frame;

    return rv;
}

template std::auto_ptr< Traits<dStorm::Image<unsigned short,3> > > 
    OpenFile::getTraits<unsigned short,3>();

OpenFile::~OpenFile() {
    TIFFClose( tiff );
}

}
}
