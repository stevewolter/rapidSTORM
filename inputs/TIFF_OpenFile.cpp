#define DSTORM_TIFFLOADER_CPP
#include <tiffio.h>
#include "TIFF.h"
#include "TIFFOperation.h"
#include <dStorm/ImageTraits.h>
#include <dStorm/ImageTraits_impl.h>
#include <boost/units/io.hpp>

#undef DEBUG
#include "debug.h"

namespace dStorm {
namespace TIFF {

static ttag_t resolution_tags[2] = { TIFFTAG_XRESOLUTION, TIFFTAG_YRESOLUTION };
static ttag_t size_tags[3] = { TIFFTAG_IMAGEWIDTH, TIFFTAG_IMAGELENGTH, TIFFTAG_IMAGEDEPTH };

OpenFile::OpenFile(const std::string& filename, const Config& config, simparm::Node& n)
: ignore_warnings(config.ignore_warnings()), determine_length(config.determine_length()),
  file_ident(filename),
  current_directory(0),
  _no_images(-1)
{
    if ( filename == test_file_name ) {
        size[0] = size[1] = size[2] = 42;
        tiff = NULL;
        return;
    }
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
            boost::optional< boost::units::quantity<camera::resolution,float> > r;
            if ( unit == RESUNIT_INCH )
                r = res * camera::pixel / (0.0254f * boost::units::si::meters);
            else if ( unit == RESUNIT_CENTIMETER )
                r = res * camera::pixel / (0.01f * boost::units::si::meters);

            if ( r.is_initialized() )
                resolution[i] = traits::ImageResolution(1.0f / *r);
        }
    }

}

template<typename Pixel, int Dim>                                   
typename std::auto_ptr< Traits<dStorm::Image<Pixel,Dim> > > 
OpenFile::getTraits( bool final, simparm::Entry<long>& n ) 
{
    if ( determine_length && final ) {
        TIFFOperation op( "in reading image count from TIFF file",
                        n, ignore_warnings );
        DEBUG("Counting images in file");
        _no_images = 1;
        int last_output[2] = { 1, 1 };
        while ( TIFFReadDirectory(tiff) != 0 ) {
            _no_images += 1;
            if ( _no_images == last_output[0] + last_output[1] ) {
                last_output[0] = last_output[1];
                last_output[1] = _no_images;
                n = _no_images;
            }
        }
#if 0 /* For some reason, this call just blocks under windows. Avoid it. */
        DEBUG("Resetting position");
        TIFFSetDirectory(tiff, 0);
#else
        DEBUG("Closing file");
        TIFFClose(tiff);
        DEBUG("Re-opening file " << file_ident.c_str());
        tiff = TIFFOpen( file_ident.c_str(), "rCm" );
        if ( tiff == NULL ) {
            DEBUG("Failed to re-open TIFF file");
            op.throw_exception_for_errors(); throw std::logic_error("Undefined error in TIFF reading"); 
        }
#endif
    }

    DEBUG("Creating traits for size " << Dim);
    BOOST_STATIC_ASSERT( Dim <= 3 );
    std::auto_ptr< Traits<dStorm::Image<Pixel,Dim> > >
        rv( new Traits<dStorm::Image<Pixel,Dim> >() );
    for (int i = 0; i < Dim; ++i) rv->size[i] = size[i] * camera::pixel;
    rv->dim = 1; /* TODO: Read from file */
    DEBUG("Setting resolutions");
    for (int p = 0; p < rv->plane_count(); ++p)
        rv->plane(p).set_resolution( resolution );
    DEBUG("Setting image range");
    for (int p = 0; p < rv->plane_count(); ++p)
    if ( _no_images != -1 )
        rv->image_number().range().second = (_no_images - 1) * camera::frame;

    DEBUG("Returning traits");
    return rv;
}

OpenFile::~OpenFile() {
    if ( tiff ) TIFFClose( tiff );
}

std::auto_ptr<BaseTraits> OpenFile::getTraits()
{
    simparm::Entry<long> unused("Foo", "Foo");
    return std::auto_ptr<BaseTraits>( getTraits<unsigned short,3>(false, unused).release() );
}

}
}
