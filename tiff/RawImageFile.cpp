#include "debug.h"
#include "tiff/RawImageFile.h"
#include <cassert>
#include "image/Image.h"
#include <boost/units/io.hpp>
#include <stdint.h>
#include "tiff/TIFFOperation.h"
#include "image/subtract.hpp"
#include <boost/scoped_array.hpp>

namespace dStorm {
namespace output {

using namespace engine;

static std::string tiff_error;

void RawImageFile::error_handler( const char* module,
                           const char* fmt, va_list ap )
{
    const int size = 4096;
    char buffer[size];
    vsnprintf( buffer, 4096, fmt, ap );
    tiff_error = buffer;
}

RawImageFile::Config::Config() 
: outputFile("ToFile", "TIF output file name", ".tif"),
  save_background("OutputType", "Output type")
{
    save_background.addChoice(new OutputTypeChoice("Signal", "Signal", Signal));
    save_background.addChoice(new OutputTypeChoice("CorrectedSignal",
                "Background-corrected signal", BackgroundCorrectedSignal));
    save_background.addChoice(new OutputTypeChoice("Background",
                "Background only", Background));
}

RawImageFile::RawImageFile(const Config& config)
: filename( config.outputFile() ),
  output_type(config.save_background().output_type),
  tif( NULL ),
  next_image(0)
{
    DEBUG("Creating TIFF output for image " << filename);
    if ( ! config.outputFile )
        throw std::runtime_error(
            "No file name supplied for raw image output");
}

Output::AdditionalData
RawImageFile::announceStormSize(const Announcement &a) 
{
    last_frame = a.image_number().range().second;
    if ( a.input_image_traits.get() ) {
        size.clear();
        for (int p = 0; p < a.input_image_traits->plane_count(); ++p) {
            size.push_back( a.input_image_traits->image(p) );
            if ( size.back().size[0] != size[0].size[0] )
                throw std::runtime_error("Only planes of equal width can be written to a TIFF file");
        }
    } else
        throw std::runtime_error("The raw images output needs access to the raw image data, but these are not provided by the preceding modules");

    for (const auto& plane : *a.input_image_traits) {
        if (output_type != Signal && !plane.has_background_estimate) {
            throw std::runtime_error("Need a background estimate to store raw images for background or background-corrected signal");
        }
    }

    TIFFOperation op("in writing TIFF file", current_ui, false);
    if ( tif == NULL ) {
        DEBUG("Opening TIFF output file");
        tif = TIFFOpen( filename.c_str(), "w" );
        if ( tif == NULL ) {
            op.throw_exception_for_errors();
        }
    }

    strip_size = TIFFTileSize( tif );
    strips_per_image = TIFFNumberOfTiles( tif );
    next_image = *a.image_number().range().first;

    return AdditionalData().set_source_image();
}

void RawImageFile::receiveLocalizations(const EngineResult& er)
{
  if ( tif == NULL ) 
    /* TIFF file got closed earlier. Return immediately. */
    return;

  try {
    if (er.source) {
        assert(er.source->frame_number() >= next_image);
        write_image(*er.source);
        next_image = er.source->frame_number() + 1 * camera::frame;
    }
    return;
  } catch ( const std::bad_alloc& a ) {
    simparm::Message m( "Out of memory while writing TIFF image",
        "The memory was exhausted while writing to the TIFF output file. The current image is dropped and not stored.",
        simparm::Message::Warning);
    m.send( current_ui );
    return;
  } catch ( const std::runtime_error& e ) {
    simparm::Message m("Error in writing TIFF file",
        std::string(e.what()) + ". Disabling TIFF output for this job.");
    m.send( current_ui );
  }

    /* When errors occured, TIFFClose tends to kill the whole program
     * unconditionally. Rather accept leaks than call it. */
    // TIFFClose( tif );
    tif = NULL;
}

void RawImageFile::write_image(const engine::ImageStack& img) {
    assert( img.has_invalid_planes() || (size[0].size.array() == img.plane(0).sizes().array()).all() );
    int lines = 0;
    for (int p = 0; p < img.plane_count(); ++p) {
        lines += img.plane(p).height_in_pixels();
    }

    TIFFOperation op("in writing TIFF file", current_ui, false);
    TIFFSetField( tif, TIFFTAG_IMAGEWIDTH, uint32_t(size[0].size.x() / camera::pixel) );
    TIFFSetField( tif, TIFFTAG_IMAGELENGTH, uint32_t(lines) );
    TIFFSetField( tif, TIFFTAG_SAMPLESPERPIXEL, 1 );
    TIFFSetField( tif, TIFFTAG_BITSPERSAMPLE, sizeof(StormPixel) * 8 );
    TIFFSetField( tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_CENTIMETER );
    if ( size[0].has_resolution(0) )
        TIFFSetField( tif, TIFFTAG_XRESOLUTION, float(size[0].resolution(0).in_dpm() * (0.01 * boost::units::si::meter) / camera::pixel) );
    if ( size[0].has_resolution(1) )
        TIFFSetField( tif, TIFFTAG_YRESOLUTION, float(size[0].resolution(1).in_dpm() * (0.01 * boost::units::si::meter) / camera::pixel) );
    if ( last_frame.is_initialized() ) {
        TIFFSetField( tif, TIFFTAG_PAGENUMBER, uint16_t(img.frame_number() / camera::frame),
                                            uint16_t(*last_frame / camera::frame + 1) );
    }
    op.throw_exception_for_errors();

    tsize_t scanline_size = TIFFScanlineSize(tif);
    char empty_scanline[scanline_size];
    std::fill( empty_scanline, empty_scanline + scanline_size, 0 );

    int current_line = 0;
    for (int p = 0; p < img.plane_count(); ++p)
    {
        engine::Image2D plane;
        switch (output_type) {
            case Signal:
                plane = img.plane(p);
                break;
            case Background:
                plane = img.background(p);
                break;
            case BackgroundCorrectedSignal:
                plane = img.plane(p) - img.background(p);
                break;
            default:
                throw std::logic_error("Unknown output type");
        }

        for (int y = 0; y < plane.height_in_pixels(); ++y)
        {
            tdata_t data;
            if ( plane.is_invalid() )
                data = empty_scanline;
            else
                data = const_cast<tdata_t>( (const tdata_t)&plane(0, y) );
            tsize_t r = TIFFWriteScanline(tif, data, current_line++, 0);
            if ( r == -1 /* Error occured */ ) 
                op.throw_exception_for_errors();
        }
    }
    if ( TIFFWriteDirectory( tif ) == 0 /* Error occured */ )
        op.throw_exception_for_errors();
}

void RawImageFile::store_results_( bool ) {
    if ( tif != NULL ) {
        DEBUG("Closing TIFF output file");
        TIFFOperation op("in closing TIFF file", current_ui, false);
        TIFFClose( tif );
        tif = NULL;
    }
}

RawImageFile::~RawImageFile() {
    if ( tif != NULL ) {
        DEBUG("Closing TIFF output file");
        TIFFOperation op("in closing TIFF file", current_ui, false);
        TIFFClose( tif );
        tif = NULL;
    }
}


}
}
