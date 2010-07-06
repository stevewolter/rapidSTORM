#include "debug.h"
#include "RawImageFile.h"
#include <cassert>
#include <dStorm/Image.h>
#include <boost/units/io.hpp>
#include <stdint.h>

namespace dStorm {
namespace output {

using namespace engine;

class RawImageFile::LookaheadImg 
{
  public:
    typedef dStorm::engine::Image Image;
  private:
    Image* image;
  public:
    LookaheadImg(Image* image) : image(image) {}
    const Image* get() const { return image; }
    frame_index image_number() const { return image->frame_number(); }
    /* Invert sense of matching to put smallest image first in
        * queue. */
    bool operator<( const LookaheadImg& o ) const
        { return o.image->frame_number() < image->frame_number(); }
};

static std::string tiff_error;

void RawImageFile::error_handler( const char* module,
                           const char* fmt, va_list ap )
{
    const int size = 4096;
    char buffer[size];
    vsnprintf( buffer, 4096, fmt, ap );
    tiff_error = buffer;
    std::cerr << "TIFF error " << buffer << std::endl;
}

RawImageFile::_Config::_Config() 
: simparm::Object("RawImage", "Save raw images"),
  outputFile("ToFile", "TIF output file name",
   ".tif")
{
}

RawImageFile::RawImageFile(const Config& config)
: OutputObject("RawImage", "Saving raw images"),
  filename( config.outputFile() ),
  tif( NULL ),
  next_image(0)
{
    if ( ! config.outputFile )
        throw std::runtime_error(
            "No file name supplied for raw image output");
}

Output::AdditionalData
RawImageFile::announceStormSize(const Announcement &a) {
    resolution = a.traits.resolution;
    last_frame = a.traits.last_frame;

    TIFFSetErrorHandler( &error_handler );
    if ( tif == NULL ) {
        tif = TIFFOpen( filename.c_str(), "w" );
        if ( tif == NULL ) 
            throw std::runtime_error("Unable to open TIFF file" + 
                                    tiff_error);
    }

    strip_size = TIFFTileSize( tif );
    strips_per_image = TIFFNumberOfTiles( tif );
    next_image = a.traits.first_frame;

    return AdditionalData().set_source_image();
}

Output::Result RawImageFile::receiveLocalizations(const EngineResult& er)
{
  ost::MutexLock lock(mutex);
  if ( tif == NULL ) 
    /* TIFF file got closed earlier. Return immediately. */
    return RemoveThisOutput;

  try {
    DEBUG("Got " << er.forImage << " while expecting " << next_image);
    /* Got the image in sequence. Write immediately. If forImage is
     * smaller, indicates engine restart and we don't need to do
     * anything, if larger, we store the image for later use. */
    if ( er.forImage == next_image ) {
            write_image( *er.source );
            while ( !out_of_time.empty() &&
                    out_of_time.top().image_number() == next_image ) 
            {
                write_image( *out_of_time.top().get() );
                delete out_of_time.top().get();
                out_of_time.pop();
            }
    } else if ( er.forImage > next_image ) {
        out_of_time.push( LookaheadImg( new LookaheadImg::Image(*er.source) ) );
    } else 
        /* Image already written. Drop. */;
    
    return KeepRunning;
  } catch ( const std::bad_alloc& a ) {
    std::cerr << "Out of memory. Dropping image from TIFF file.\n";
    return KeepRunning;
  } catch ( const std::exception& e ) {
    std::cerr << e.what() << ". Disabling TIFF output.\n";
  }

    /* If we got here, we had an exception. */
    delete_queue();
    /* When errors occured, TIFFClose tends to kill the whole program
     * unconditionally. Rather accept leaks than call it. */
    // TIFFClose( tif );
    tif = NULL;
    return RemoveThisOutput;
}

void RawImageFile::delete_queue() {
    while ( ! out_of_time.empty() ) {
        delete out_of_time.top().get();
        out_of_time.pop();
    }
}

void RawImageFile::write_image(const dStorm::engine::Image& img) {
    DEBUG("Writing " << img.frame_number().value());
    TIFFSetField( tif, TIFFTAG_IMAGEWIDTH, img.width_in_pixels() );
    TIFFSetField( tif, TIFFTAG_IMAGELENGTH, img.height_in_pixels() );
    TIFFSetField( tif, TIFFTAG_SAMPLESPERPIXEL, 1 );
    TIFFSetField( tif, TIFFTAG_BITSPERSAMPLE, sizeof(StormPixel) * 8 );
    if ( resolution.is_set() ) {
        TIFFSetField( tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_CENTIMETER );
        TIFFSetField( tif, TIFFTAG_XRESOLUTION, int(*resolution * (0.01 * boost::units::si::meter) / cs_units::camera::pixel) );
        TIFFSetField( tif, TIFFTAG_YRESOLUTION, int(*resolution * (0.01 * boost::units::si::meter) / cs_units::camera::pixel) );
    }
    if ( last_frame.is_set() ) {
        TIFFSetField( tif, TIFFTAG_PAGENUMBER, uint16_t(img.frame_number() / cs_units::camera::frame),
                                               uint16_t(*last_frame / cs_units::camera::frame + 1) );
    }

    strip_size = TIFFStripSize( tif );
    tstrip_t number_of_strips = TIFFNumberOfStrips( tif );
    tdata_t data = const_cast<tdata_t>( (const tdata_t)img.ptr() );
    for ( tstrip_t strip = 0; strip < number_of_strips; strip++ ) {
        tsize_t r = TIFFWriteRawStrip(tif, strip, data, strip_size);
        if ( r == -1 /* Error occured */ ) 
            throw std::runtime_error("Writing TIFF failed: " + tiff_error);
        assert( sizeof(char) == 1 );
        data= ((char*)data) +strip;
    }
    if ( TIFFWriteDirectory( tif ) == 0 /* Error occured */ )
        throw std::runtime_error("Writing TIFF failed: " + tiff_error);
    next_image = next_image + 1 * cs_units::camera::frame;
}

void RawImageFile::propagate_signal(ProgressSignal) {
}

RawImageFile::~RawImageFile() {
    delete_queue();
    if ( tif != NULL ) {
        DEBUG("Closing TIFF output file");
        TIFFClose( tif );
    }
}


}
}
