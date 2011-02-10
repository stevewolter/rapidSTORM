#include <dStorm/helpers/exception.h>
#include "debug.h"
#include "RawImageFile.h"
#include <cassert>
#include <dStorm/Image.h>
#include <boost/units/io.hpp>
#include <stdint.h>
#include "TIFFOperation.h"
#include <boost/scoped_array.hpp>

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
    DEBUG("Creating TIFF output for image " << filename);
    if ( ! config.outputFile )
        throw std::runtime_error(
            "No file name supplied for raw image output");
}

Output::AdditionalData
RawImageFile::announceStormSize(const Announcement &a) 
{
    last_frame = a.image_number().range().second;
    if ( a.input_image_traits.get() )
        size = *a.input_image_traits;
    else
        throw std::runtime_error("The raw images output needs access to the raw image data, but these are not provided by the preceding modules");

    TIFFOperation op("in writing TIFF file", *this, false);
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
    simparm::Message m( "Out of memory while writing TIFF image",
        "The memory was exhausted while writing to the TIFF output file. The current image is dropped and not stored.",
        simparm::Message::Warning);
    this->send( m );
    return KeepRunning;
  } catch ( const dStorm::exception& e ) {
    simparm::Message m = e.get_message("Disabling TIFF output");
    this->send( m );
  } catch ( const std::exception& e ) {
    simparm::Message m("Error in writing TIFF file",
        std::string(e.what()) + ". Disabling TIFF output for this job.");
    this->send( m );
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
    TIFFOperation op("in writing TIFF file", *this, false);
    DEBUG("Writing " << img.frame_number().value() << " of size " << size.size.transpose());
    TIFFSetField( tif, TIFFTAG_IMAGEWIDTH, uint32_t(size.size.x() / camera::pixel) );
    TIFFSetField( tif, TIFFTAG_IMAGELENGTH, uint32_t(size.size.y() / camera::pixel) );
    TIFFSetField( tif, TIFFTAG_IMAGEDEPTH, uint32_t(size.size.z() / camera::pixel) );
    TIFFSetField( tif, TIFFTAG_SAMPLESPERPIXEL, 1 );
    TIFFSetField( tif, TIFFTAG_BITSPERSAMPLE, sizeof(StormPixel) * 8 );
    TIFFSetField( tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_CENTIMETER );
    if ( size.resolution[0].is_set() )
        TIFFSetField( tif, TIFFTAG_XRESOLUTION, int(size.resolution[0]->in_dpm() * (0.01 * boost::units::si::meter) / camera::pixel) );
    if ( size.resolution[1].is_set() )
        TIFFSetField( tif, TIFFTAG_YRESOLUTION, int(size.resolution[1]->in_dpm() * (0.01 * boost::units::si::meter) / camera::pixel) );
    if ( last_frame.is_set() ) {
        TIFFSetField( tif, TIFFTAG_PAGENUMBER, uint16_t(img.frame_number() / camera::frame),
                                            uint16_t(*last_frame / camera::frame + 1) );
    }
    op.throw_exception_for_errors();

    strip_size = TIFFStripSize( tif );
    tstrip_t number_of_strips = TIFFNumberOfStrips( tif );
    if ( ! img.is_invalid() ) {
        tdata_t data = const_cast<tdata_t>( (const tdata_t)img.ptr() );
        for ( tstrip_t strip = 0; strip < number_of_strips; strip++ ) {
            tsize_t r = TIFFWriteRawStrip(tif, strip, data, strip_size);
            if ( r == -1 /* Error occured */ ) 
                op.throw_exception_for_errors();
            assert( sizeof(char) == 1 );
            data= ((char*)data) +strip;
        }
    } else {
        /* If the image is invalid, write empty image data */
        boost::scoped_array<StormPixel> nulls( new StormPixel[strip_size] );
        for (int i = 0; i < strip_size; ++i ) nulls[i] = 0;
        for ( tstrip_t strip = 0; strip < number_of_strips; strip++ ) {
            tsize_t r = TIFFWriteRawStrip(tif, strip, nulls.get(), strip_size);
            if ( r == -1 /* Error occured */ ) 
                op.throw_exception_for_errors();
        }
    }
    if ( TIFFWriteDirectory( tif ) == 0 /* Error occured */ )
        op.throw_exception_for_errors();
    next_image = next_image + 1 * camera::frame;
}

void RawImageFile::propagate_signal(ProgressSignal) {
}

RawImageFile::~RawImageFile() {
    delete_queue();
    if ( tif != NULL ) {
        DEBUG("Closing TIFF output file");
        TIFFOperation op("in closing TIFF file", *this, false);
        TIFFClose( tif );
    }
}


}
}
