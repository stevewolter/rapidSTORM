#define cimg_use_magick
#include "debug.h"
#include "plugin.h"
#include <stdint.h>
#include "Viewer.h"
#include "ViewerConfig.h"
#include <limits>
#include <cassert>
#include <dStorm/engine/Image.h>
#include <CImg.h>
#include <fstream>
#include "doc/help/context.h"

#include <dStorm/outputs/BinnedLocalizations.h>
#include <dStorm/outputs/BinnedLocalizations_impl.h>
#include "ImageDiscretizer_inline.h"
#include "ColourDisplay_impl.h"
#include "Display_inline.h"

#ifndef UINT8_MAX
#define UINT8_MAX std::numeric_limits<uint8_t>::max()
#endif

#include <dStorm/helpers/DisplayManager.h>

#include "config.h"

using namespace std;
using namespace cimg_library;
using namespace ost;

using namespace dStorm::Display;
using namespace dStorm::outputs;
using namespace dStorm::output;

namespace dStorm {
namespace outputs {

void add_viewer( output::Config& config ) {
    config.addChoice( new viewer::Viewer::Source() );
}

}
}

static Mutex *cimg_lock = NULL;

static const char *SI_prefixes[]
= { "f", "p", "n", "µ", "m", "", "k", "M", "G", "T",
    "E" };
std::string SIize( float value ) {
    if ( value < 1E-21 ) return "0";
    int prefix = int(floor(log10(value))) / 3;
    prefix = std::max(-5, std::min(prefix, 5));
    float rv = value / pow(1000, prefix);
    char buffer[128];
    snprintf( buffer, 128, "%.3g %s", rv, 
              SI_prefixes[prefix + 5] );
    return buffer;
}

namespace Eigen {
bool operator<( const Eigen::Vector2i& a, const Eigen::Vector2i& b )
{
    if ( a.x() < b.x() )
        return true;
    else if ( a.x() > b.x() )
        return false;
    else
        return a.y() < b.y();
}
}

namespace dStorm {
namespace viewer {

/** Time to wait for user input in milliseconds. */
const int waitTime = 10;

struct Viewer::Implementation
{
    virtual ~Implementation() {}
    virtual output::Output& getForwardOutput() = 0;

#ifdef HAVE_LIBGRAPHICSMAGICK__
    virtual void write_full_size_image(const char *filename, bool) = 0;
#endif

    virtual void set_histogram_power(float power) = 0;
    virtual void set_resolution_enhancement(float re) = 0;

};

template <int Hueing>
class ColourDependantImplementation 
: public Viewer::Implementation,
  public DataSource
{
    typedef HueingColorizer<Hueing> MyColorizer;
    typedef Display< MyColorizer > ColouredImageAdapter;
    typedef DiscretizedImage::ImageDiscretizer< 
        MyColorizer, ColouredImageAdapter>
        Discretizer;
    typedef BinnedLocalizations<Discretizer> Accumulator;

    std::auto_ptr<Change> get_changes() {
        ost::MutexLock lock( image.getMutex() );
        image.clean(); 
        return cia.get_changes();
    }

    /** Binned image with all localizations in localizationsStore.*/
    Accumulator image;
    MyColorizer colorizer;
    /** Discretized version of \c image. */
    Discretizer discretization;
    ColouredImageAdapter cia;

  public:
    ColourDependantImplementation(const Viewer::Config& config)
        : image( config.res_enh(), 1 * cs_units::camera::pixel ),
          colorizer(config),
          discretization( 4096, 
                config.histogramPower(), image(),
                colorizer),
          cia( discretization, config, *this )
    {
        image.setListener(&discretization);
        discretization.setListener(&cia);
    }

    ~ColourDependantImplementation() {
        DEBUG("Destructing viewer implementation");
    }

    output::Output& getForwardOutput() { return image; }
#ifdef HAVE_LIBGRAPHICSMAGICK__
    virtual void write_full_size_image(const char *filename, bool with_key) 
    { 
      image.clean();
      int width = image.width();
      if ( with_key ) {
        std::auto_ptr< Magick::Image > key_img
            = discretization.key_image();
        int lh = key_img->fontPointsize(),
            text_area_height = 5 * lh;
        int key_width = width - 2 * lh;
        int real_key_width = key_img->columns();
        Magick::Geometry key_img_size 
            = Magick::Geometry(key_width, 20);
        key_img_size.aspect( true );
        key_img->sample( key_img_size );

        int image_height_with_key = this->image.height() + 
                text_area_height + 2 * lh + key_img->rows();
        Pixel bg = discretization.get_background();
        Magick::ColorRGB bgc ( bg.red()/255.0, bg.green()/255.0, 
                               bg.blue()/255.0 );
        Magick::ColorRGB bgi ( 1.0 - bgc.red(), 1.0 - bgc.green(),
                               1.0 - bgc.blue() );
        Magick::Image image
            ( Magick::Geometry(width, image_height_with_key), bgc );
        image.type(Magick::TrueColorType);
        image.write( filename );
        discretization.write_full_image( image, 0, 0 );

        int ys = this->image.height() + 10;
        image.composite( *key_img, lh, ys,
                        Magick::OverCompositeOp );

        ys += key_img->rows();
        Magick::TypeMetric metrics;
        image.strokeColor( bgi );
        image.fillColor( bgi );
        for (unsigned int i = 0;
             i < key_img->columns(); i += lh )
        {
            float value = 
                discretization.key_value(
                    i * real_key_width * 1.0 / key_width);
            std::string s = SIize(value);
            image.annotate(s, 
                Magick::Geometry( lh, text_area_height-5,
                                  i+lh/2, ys+5),
                Magick::NorthWestGravity, 90 );
            image.draw( Magick::DrawableLine( i+2*lh/3, ys, i+2*lh/3, ys + 3 ) );
        }

        ys += text_area_height;
        std::string message =
            "Key: total A/D counts per pixel";
        image.annotate( message,
            Magick::Geometry( image.columns(), 
                2*lh, 0, ys ),
                Magick::NorthGravity, 0);
        image.draw( Magick::DrawableRectangle( 
            image.columns()-105, ys-12, 
            image.columns()-5, ys-7 ) );

        image.annotate(
            SIize(cia.nm_per_pixel * 1E-7) + "m", 
                Magick::Geometry(100, lh, 
                    image.columns()-105, ys+4),
                Magick::NorthGravity );
        image.resolutionUnits( Magick::PixelsPerCentimeterResolution );
        unsigned int pix_per_cm = int( round(1E7 / cia.nm_per_pixel) );
        image.density(Magick::Geometry(pix_per_cm, pix_per_cm));
        image.write( filename );
      } else {
        Pixel bg = discretization.get_background();
        Magick::ColorRGB bgc ( bg.red()/255.0, bg.green()/255.0, 
                               bg.blue()/255.0 );
        Magick::Image image
            ( Magick::Geometry(width, this->image.height()), bgc );
        image.type(Magick::TrueColorType);
        image.write( filename );
        discretization.write_full_image( image, 0, 0 );
        image.resolutionUnits( Magick::PixelsPerCentimeterResolution );
        unsigned int pix_per_cm = int( round(1E7 / cia.nm_per_pixel) );
        image.density(Magick::Geometry(pix_per_cm, pix_per_cm));
        image.write( filename );
      }
    }
#endif

    virtual void set_histogram_power(float power) {
        /* The \c image member is not involved here, so we have to lock
         * it ourselves. */
        ost::MutexLock lock( image.getMutex() );
        discretization.setHistogramPower( power ); 
    }
    virtual void set_resolution_enhancement(float re) { 
        image.set_resolution_enhancement( re ); 
    }
};

Viewer::_Config::_Config()
: simparm::Object("Image", "Image display"),
  showOutput("ShowOutput", "Display dSTORM result image"),
  outputFile("ToFile", "Save image to", ".jpg"),
  res_enh("ResEnhance", "Resolution Enhancement", 10),
  refreshCycle("ImageRefreshCycle", "Refresh image every x ms:", 100),
  histogramPower("HistogramPower", "Extent of histogram normalization",
                 0.3),
  colourScheme("ColourScheme", "Colour palette for display"),
  hue("Hue", "Select color hue", 0),
  saturation("Saturation", "Select saturation", 1),
  invert("InvertColours", "Invert colours", false),
  save_with_key("SaveWithKey", "Save output image with key", true),
  close_on_completion("CloseOnCompletion", 
                      "Close display on job completion")
{
    PROGRESS("Building Viewer Config");

    outputFile.helpID = HELP_Viewer_ToFile;
    outputFile.setUserLevel(simparm::Entry::Beginner);
    showOutput.helpID = HELP_Viewer_ShowOutput;
    showOutput.setUserLevel(simparm::Entry::Beginner);
    res_enh.helpID = HELP_Viewer_ResEnh;
    res_enh.setHelp("The target image will have a resolution this many "
                    "times higher than the source image. For example, "
                    "a 128x128 image with resolution 10 will result in "
                    "a 1280x1280 sized image.");
    res_enh.setUserLevel(simparm::Entry::Beginner);
    res_enh.setMin(1);
    refreshCycle.setUserLevel(simparm::Entry::Intermediate);

    histogramPower.setMin(0);
    histogramPower.setMax(1);
    /* This level is reset in carStarted() */
    histogramPower.setUserLevel(simparm::Entry::Expert);

    colourScheme.helpID = HELP_Viewer_ColorScheme;
    colourScheme.addChoice(ColourSchemes::BlackWhite, "BlackWhite", 
        "Black and white");
    colourScheme.addChoice(ColourSchemes::BlackRedYellowWhite,
        "BlackRedYellowWhite", 
        "Colour code ranging from red over yellow to white");
    colourScheme.addChoice(ColourSchemes::FixedHue, "FixedHue", 
        "Constant colour given by hue and variance");
    colourScheme.addChoice(ColourSchemes::TimeHue,
        "HueByTime", "Vary hue by time coordinate");
    colourScheme.addChoice( ColourSchemes::ExtraHue,
        "HueByCharacteristic", "Vary hue by characteristic");
    colourScheme.addChoice( ColourSchemes::ExtraSaturation,
        "SaturationByCharacteristic", "Vary saturation by characteristic");

    colourScheme = ColourSchemes::BlackRedYellowWhite;

    hue.helpID = HELP_Viewer_Hue;
    hue.setMin(0);
    hue.setMax(1);
    hue.setHelp("Select a hue between 0 and 1 to display localizations in."
                " The hue is selected along the HSV color axis, following "
                "the natural spectrum from 0 (red) over 1/6 (yellow), "
                "1/3 (green), 1/2 (cyan), 2/3 (blue) to 5/6 (violet) and "
                "1 (red again)");
    saturation.helpID = HELP_Viewer_Saturation;
    saturation.setMin(0);
    saturation.setMax(1);
    saturation.setHelp("Select a saturation between 0 and 1 for the color "
                       "in the display. Saturation 0 means no color (pure "
                       "black to pure white) and 1 means fully saturated "
                       "color.");

    invert.helpID = HELP_Viewer_InvertColors;

    close_on_completion.setUserLevel(simparm::Entry::Debug);
    save_with_key.setUserLevel(simparm::Entry::Intermediate);

    PROGRESS("Built Viewer Config");
}

void Viewer::_Config::registerNamedEntries() {
   push_back(outputFile);
   push_back(save_with_key);
   push_back(showOutput);
   push_back(res_enh);
   push_back(histogramPower);
   push_back(colourScheme);
   push_back(invert);
   push_back(hue);
   push_back(saturation);
   push_back(close_on_completion);
}

template <int Index>
static Viewer::Implementation* make_binned_locs(
    const Viewer::Config& config
) {
    int index = ( config.colourScheme.value() )();

    if ( index == Index ) {
        return new ColourDependantImplementation<Index>( config );
    } else if ( index >= ColourSchemes::FirstColourModel
                && index <= Index ) {
        return make_binned_locs< 
            (Index > ColourSchemes::FirstColourModel) ? Index-1 : Index >
                ( config );
    } else {
        throw std::logic_error("Invalid colour scheme.");
    }
}

Viewer::Viewer(const Viewer::Config& config)
: OutputObject("Display", "Display status"),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  implementation( 
    make_binned_locs< ColourSchemes::LastColourModel >( config ) ),
  forwardOutput( implementation->getForwardOutput() ),
  runViewer( config.showOutput() ),
  runningViewer(false),
  terminateViewer(false),
  needsResizing(false),
  isEmpty(true),
  close_display_immediately( config.close_on_completion() ),
  cyclelength( config.refreshCycle() / waitTime ),
  tifFile( "ToFile", "Save image to", config.outputFile() ),
  save_with_key( config.save_with_key ),
  resolutionEnhancement( config.res_enh ),
  histogramPower( config.histogramPower ),
  save("SaveImage", "Save image"),
  quit("Quit", "Close viewing window")
{
    if (cimg_lock == NULL) cimg_lock = new Mutex();

    resolutionEnhancement.helpID = HELP_Viewer_Status_ResEnh;
    histogramPower.helpID = HELP_Viewer_Status_Power;
    tifFile.helpID = HELP_Viewer_Status_ToFile;
    save.helpID = HELP_Viewer_Status_Save;

    /* With the values provided in config, meaningful defaults can
     * be set in the following config entries. */
    histogramPower.setUserLevel(simparm::Entry::Beginner);
    save.setUserLevel(simparm::Entry::Beginner);

    push_back( resolutionEnhancement );
    push_back( histogramPower );
    push_back( tifFile );
    push_back( save );
    // push_back( quit ); /* quit is pushed back in the subthread */

    receive_changes_from( save.value );
    receive_changes_from( quit.value );
    receive_changes_from( histogramPower.value );
    receive_changes_from( resolutionEnhancement.value );

    if (config.showOutput() ) {
#if 0
        string name = config.getDesc();
        int initial_width = 4*CImgDisplay::screen_dimx()/5,
            initial_height = 4*CImgDisplay::screen_dimy()/5;
#endif
    }
}

Viewer::~Viewer() {
    DEBUG("Destructing Viewer");
}


Output::Result
Viewer::receiveLocalizations(const EngineResult& er)
{
    if ( er.number > 0 ) isEmpty = false;
    MutexLock lock(structureMutex);
    return forwardOutput.receiveLocalizations(er);
}

Output::AdditionalData 
Viewer::announceStormSize(const Announcement &a) {
    MutexLock lock(structureMutex);
    return forwardOutput.announceStormSize(a);
}

void Viewer::propagate_signal(ProgressSignal s) {
    {
        MutexLock lock(structureMutex);
        forwardOutput.propagate_signal(s);
    }

    if ( s == Engine_is_restarted || s == Engine_run_failed )
        isEmpty = true;
    else if (s == Engine_run_succeeded && tifFile) {
#ifdef HAVE_LIBGRAPHICSMAGICK__
        writeToFile(tifFile());
#endif
    }
}

void Viewer::operator()(const simparm::Event& e) {
    if (&e.source == &save.value && save.triggered()) {
        /* Save image */
        save.untrigger();
        if ( tifFile ) {
#ifdef HAVE_LIBGRAPHICSMAGICK__
            writeToFile( tifFile() );
#else
	    throw std::logic_error(PACKAGE_NAME " was compiled without "
                "Magick++ support and therefore can not save images.");
#endif
        }
    } else if (&e.source == &quit.value && quit.triggered()) {
        /* Close viewer */
        quit.untrigger();
        terminateViewer = true;
    } else if (&e.source == &histogramPower.value) {
        MutexLock lock(structureMutex);
        /* Change histogram power */
        implementation->set_histogram_power(histogramPower());
    } else if (&e.source == &resolutionEnhancement.value) {
        MutexLock lock(structureMutex);
        /* Change resolution enhancement in viewer */
        implementation->
            set_resolution_enhancement( resolutionEnhancement() );
    } 
}

#ifdef HAVE_LIBGRAPHICSMAGICK__
void Viewer::writeToFile(const string &name) {
    try {
        MutexLock lock(structureMutex);

        const char * const filename = name.c_str();
        implementation->write_full_size_image(filename, save_with_key());
    } catch ( const std::exception& e ) {
        std::cerr << "Writing image failed: " << e.what() << endl;
    }
}
#endif

void Viewer::check_for_duplicate_filenames
        (std::set<std::string>& present_filenames)
{
    insert_filename_with_check( tifFile(), present_filenames );
}

}
}
