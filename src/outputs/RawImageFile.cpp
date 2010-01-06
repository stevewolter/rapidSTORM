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
  outputFile("ToFile", "TIF output file name")
{
    outputFile.default_extension = ".tif";
}

RawImageFile::RawImageFile(const Config& config)
: OutputObject("RawImage", "Saving raw images"),
  next_image(0)
{
    if ( ! config.outputFile )
        throw std::runtime_error(
            "No file name supplied for raw image output");
    TIFFSetErrorHandler( &error_handler );
    std::string filename = config.outputFile();
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
    }
    
    return KeepRunning;
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
        TIFFWriteRawStrip(tif, strip, data, strip_size);
        assert( sizeof(char) == 1 );
        data= ((char*)data) +strip;
    }
    TIFFWriteDirectory( tif );
    next_image = next_image + 1 * camera::frame;
}

void RawImageFile::propagate_signal(ProgressSignal) {
}

RawImageFile::~RawImageFile() {
    TIFFClose( tif );
    while ( ! out_of_time.empty() ) {
        delete out_of_time.top().get();
        out_of_time.pop();
    }
}


}
}
