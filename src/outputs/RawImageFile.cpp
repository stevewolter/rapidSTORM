#include "RawImageFile.h"
#include <cassert>
#include <CImg.h>

namespace dStorm {
namespace output {

using namespace engine;

class RawImageFile::LookaheadImg 
{
    frame_index num;
    Image* image;
  public:
    LookaheadImg(frame_index imageNumber, Image* image)
        : num(imageNumber), image(image) {}
    const Image* get() const { return image; }
    frame_index image_number() const { return num; }
    /* Invert sense of matching to put smallest image first in
        * queue. */
    bool operator<( const LookaheadImg& o ) const
        { return o.num < num; }
};

static std::string tiff_error;

void RawImageFile::error_handler( const char* module,
                           const char* fmt, va_list ap )
{
    const int size = 4096;
    char buffer[size];
    vsnprintf( buffer, 4096, fmt, ap );
    tiff_error = buffer;
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
  next_image(0)
{
    if ( ! config.outputFile )
        throw std::runtime_error(
            "No file name supplied for raw image output");
    TIFFSetErrorHandler( &error_handler );
    tif = TIFFOpen( filename.c_str(), "w" );
    if ( tif == NULL ) 
        throw std::runtime_error("Unable to open TIFF file" + 
                                 tiff_error);
}

Output::AdditionalData
RawImageFile::announceStormSize(const Announcement &) {
    strip_size = TIFFTileSize( tif );
    strips_per_image = TIFFNumberOfTiles( tif );

    return AdditionalData().set_source_image();
}

Output::Result RawImageFile::receiveLocalizations(const EngineResult& er)
{
  ost::MutexLock lock(mutex);
  if ( tif == NULL ) 
    /* TIFF file got closed earlier. Return immediately. */
    return RemoveThisOutput;

  try {
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
        out_of_time.push( LookaheadImg( er.forImage, 
                                             new Image(*er.source) ) );
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

void RawImageFile::write_image(const Image& img) {
    TIFFSetField( tif, TIFFTAG_IMAGEWIDTH, img.width );
    TIFFSetField( tif, TIFFTAG_IMAGELENGTH, img.height );
    TIFFSetField( tif, TIFFTAG_SAMPLESPERPIXEL, 1 );
    TIFFSetField( tif, TIFFTAG_BITSPERSAMPLE, sizeof(StormPixel) * 8 );

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
    if ( tif != NULL )
        TIFFClose( tif );
}


}
}
