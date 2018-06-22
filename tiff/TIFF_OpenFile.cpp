#define DSTORM_TIFFLOADER_CPP
#include <tiffio.h>
#include "tiff/OpenFile.h"
#include "tiff/TIFFOperation.h"
#include "engine/InputTraits.h"
#include "engine/Image.h"
#include "image/MetaInfo.h"
#include "image/convert.h"
#include <boost/units/io.hpp>
#include <boost/smart_ptr/scoped_array.hpp>
#include "tiff/TIFF.h"

#undef DEBUG
#include "debug.h"

namespace dStorm {
namespace tiff {

static ttag_t resolution_tags[2] = { TIFFTAG_XRESOLUTION, TIFFTAG_YRESOLUTION };
static ttag_t size_tags[3] = { TIFFTAG_IMAGEWIDTH, TIFFTAG_IMAGELENGTH, TIFFTAG_IMAGEDEPTH };

OpenFile::OpenFile(const std::string& filename, const Config& config, simparm::NodeHandle n)
: ignore_warnings(config.ignore_warnings()), determine_length(config.determine_length()),
  file_ident(filename),
  current_directory(0),
  _no_images(-1)
{
    if ( filename == test_file_name ) {
        size.fill( 42 * camera::pixel );
        tiff = NULL;
        return;
    }
    size.fill(0 * camera::pixel);
    TIFFOperation op( "in opening TIFF file",
                      n, ignore_warnings );
    tiff = TIFFOpen( filename.c_str(), "rCm" );
    if ( tiff == NULL ) { op.throw_exception_for_errors(); throw std::logic_error("Undefined error in TIFF reading"); }

    for (int i = 0; i < Dim; ++i ) {
        uint32_t field;
        if ( ! TIFFGetField( tiff, size_tags[i], &field ) )
            field = 1;
        size[i] = field * camera::pixel;
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

std::auto_ptr< input::Traits<engine::ImageStack > > 
OpenFile::getTraits( bool final, simparm::Entry<long>& n ) 
{
    if ( determine_length && final ) {
        TIFFOperation op( "in reading image count from TIFF file",
                        n.get_user_interface_handle(), ignore_warnings );
        DEBUG("Counting images in file");
        _no_images = 1;
        int last_output[2] = { 1, 1 };
        n.show();
        while ( TIFFReadDirectory(tiff) != 0 ) {
            _no_images += 1;
            if ( _no_images == last_output[0] + last_output[1] ) {
                last_output[0] = last_output[1];
                last_output[1] = _no_images;
                n = _no_images;
            }
        }
#if 0 /* For some reason, this call just blocks under windows. Avoid it. */
        TIFFSetDirectory(tiff, 0);
#else
        TIFFClose(tiff);
        tiff = TIFFOpen( file_ident.c_str(), "rCm" );
        if ( tiff == NULL ) {
            op.throw_exception_for_errors(); throw std::logic_error("Undefined error in TIFF reading"); 
        }
#endif
    } else {
        n.hide();
    }

    static const int Dim = engine::ImageStack::Plane::Dim;
    DEBUG("Creating traits for size " << Dim);
    BOOST_STATIC_ASSERT( Dim <= 3 );
    std::auto_ptr< input::Traits<engine::ImageStack > > rv
        ( new input::Traits<engine::ImageStack >() );
    for (int z = 0; z < size[2].value(); ++z) {
        image::MetaInfo<2> plane;
        plane.size = size.head<2>();
        DEBUG("Setting resolutions");
        plane.set_resolution( resolution );
        DEBUG("Setting image range");
        rv->push_back( plane, traits::Optics() );
    }

    DEBUG("Returning traits");
    rv->image_number().range().first = 0 * camera::frame;
    if ( _no_images != -1 )
        rv->image_number().range().second = (_no_images - 1) * camera::frame;
    return rv;
}

OpenFile::~OpenFile() {
    if ( tiff ) TIFFClose( tiff );
}

std::auto_ptr<input::BaseTraits> OpenFile::getTraits()
{
    simparm::Entry<long> unused("Foo", "Foo", 0);
    return std::auto_ptr<input::BaseTraits>( getTraits(false, unused).release() );
}

void OpenFile::seek_to_image( simparm::NodeHandle msg, int directory) {
    TIFFOperation op( "in reading TIFF file",
                        msg, ignore_warnings );
    if ( current_directory != directory ) {
        int rv = TIFFSetDirectory(tiff, directory);
        if ( rv == 1 ) {
            current_directory = directory;
        } else {
            op.throw_exception_for_errors();
        }
    }
}

bool OpenFile::next_image( simparm::NodeHandle msg ) {
    TIFFOperation op( "in reading TIFF file",
                        msg, ignore_warnings );
    if ( TIFFReadDirectory(tiff) != 1 ) {
        op.throw_exception_for_errors();
        /* Code from here only executed when no error
            * was encountered */
        return false;
    } else {
        ++current_directory;
        return true;
    }
}

template <typename PixelType>
void OpenFile::read_data_(Image& i, TIFFOperation& op ) const
{
    const tsize_t strip_size = TIFFStripSize( tiff );
    const tstrip_t strip_count = TIFFNumberOfStrips( tiff );
    const int pixels_per_strip = strip_size / sizeof(PixelType);

    boost::scoped_array<PixelType> raw_data( new PixelType[ pixels_per_strip ] );
    
    Pixel* data = i.ptr(), *end_of_data = data + i.size_in_pixels();
    for (tstrip_t strip = 0; strip < strip_count; strip++) {
        const int remaining_pixels = (end_of_data - data);
        const tsize_t remaining_space = sizeof(PixelType) * remaining_pixels;
        const tsize_t to_read = std::min( remaining_space, strip_size );
        const tsize_t read_bytes = 
            TIFFReadEncodedStrip( tiff, strip, raw_data.get(), to_read );
        if ( read_bytes == -1 ) {
            op.throw_exception_for_errors();
            throw std::runtime_error("Unknown error in reading TIFF file");
        }
        const int read_pixels = read_bytes / sizeof(PixelType);
        assert( read_pixels <= pixels_per_strip );
        assert( data + read_pixels <= end_of_data );
        std::copy( raw_data.get(), raw_data.get() + read_pixels, data );
        data += read_pixels;
    }
}

OpenFile::Image OpenFile::read_image( simparm::NodeHandle msg ) const
{
    TIFFOperation op( "in reading TIFF file",
                        msg, ignore_warnings );
    uint16_t bitspersample, colors, sampleformat = SAMPLEFORMAT_VOID;
    TIFFGetField( tiff, TIFFTAG_BITSPERSAMPLE, &bitspersample );
    TIFFGetField( tiff, TIFFTAG_SAMPLEFORMAT, &sampleformat );
    if ( TIFFGetField( tiff, TIFFTAG_SAMPLESPERPIXEL, &colors ) && colors != 1 ) {
        std::stringstream error;
        error << "TIFF image no. " << TIFFCurrentDirectory(tiff)
            << " has " << colors << " color channels, but only greyscale images can be processed. Aborting.";
        throw std::runtime_error( error.str() );
    }

    Image::Size sz;
    sz.fill( 1 * camera::pixel );
    TIFFGetField( tiff, TIFFTAG_IMAGEWIDTH, &quantity_cast<int&>(sz[0]) );
    TIFFGetField( tiff, TIFFTAG_IMAGELENGTH, &quantity_cast<int&>(sz[1]) );
    TIFFGetField( tiff, TIFFTAG_IMAGEDEPTH, &quantity_cast<int&>(sz[2]) );

    if ( (sz != size).any() ) {
        std::stringstream error;
        error << "TIFF image no. " << TIFFCurrentDirectory(tiff)
            << " has dimensions (" << sz.transpose()
            << ") different from first image (" << size.transpose()
            << "). Aborting.";
        throw std::runtime_error( error.str() );
    }

    Image result( sz );

    if ( sampleformat == SAMPLEFORMAT_VOID )
        /* Try unsigned int and hope for the best. */
        sampleformat = SAMPLEFORMAT_UINT;

    if ( bitspersample == 1 && sampleformat == SAMPLEFORMAT_UINT )
        read_data_<bool>( result, op );
    else if ( bitspersample == 8 && sampleformat == SAMPLEFORMAT_UINT )
        read_data_<uint8_t>( result, op );
    else if ( bitspersample == 16 && sampleformat == SAMPLEFORMAT_UINT )
        read_data_<uint16_t>( result, op );
    else if ( bitspersample == 32 && sampleformat == SAMPLEFORMAT_UINT )
        read_data_<uint32_t>( result, op );
    else if ( bitspersample == 64 && sampleformat == SAMPLEFORMAT_UINT )
        read_data_<uint64_t>( result, op );
    else if ( bitspersample == 8 && sampleformat == SAMPLEFORMAT_INT )
        read_data_<int8_t>( result, op );
    else if ( bitspersample == 16 && sampleformat == SAMPLEFORMAT_INT )
        read_data_<int16_t>( result, op );
    else if ( bitspersample == 32 && sampleformat == SAMPLEFORMAT_INT )
        read_data_<int32_t>( result, op );
    else if ( bitspersample == 64 && sampleformat == SAMPLEFORMAT_INT )
        read_data_<int64_t>( result, op );
    else if ( bitspersample == sizeof(float)*8 && sampleformat == SAMPLEFORMAT_IEEEFP )
        read_data_<float>( result, op );
    else if ( bitspersample == sizeof(double)*8 && sampleformat == SAMPLEFORMAT_IEEEFP )
        read_data_<double>( result, op );
    else {
        std::stringstream error;
        error << "TIFF image " << current_directory << " in file " << file_ident
              << " has the unsupported bit depth " << bitspersample << ". Aborting.";
        throw std::runtime_error( error.str() );
    }

    op.throw_exception_for_errors();
    DEBUG("Thrown no exceptions");

    return result;
}

}
}
