#define cimg_use_magick
#include <stdint.h>
#include "Viewer.h"
#include "ViewerConfig.h"
#include <limits>
#include <cassert>
#include <dStorm/Image.h>
#include <CImg.h>
#include <fstream>
#include "help_context.h"

#include <outputs/BinnedLocalizations.h>
#include <outputs/BinnedLocalizations_impl.h>
#include <outputs/ImageDiscretizer.h>
#include <outputs/ColourDisplay.h>
#include <outputs/ImageDiscretizer_impl.h>
#include <outputs/ColourDisplay_impl.h>

#ifndef UINT8_MAX
#define UINT8_MAX std::numeric_limits<uint8_t>::max()
#endif

#include "DisplayHandler.h"

using namespace std;
using namespace cimg_library;
using namespace ost;

static Mutex *cimg_lock = NULL;

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

/** Time to wait for user input in milliseconds. */
const int waitTime = 10;

struct Viewer::Implementation
{
    virtual ~Implementation() {}
    virtual dStorm::Output& getForwardOutput() = 0;

    virtual std::auto_ptr< cimg_library::CImg<unsigned char> >
        full_size_image() = 0;
    virtual void wait_for_completion() = 0;

    virtual void set_histogram_power(float power) = 0;
    virtual void set_resolution_enhancement(float re) = 0;

};

template <int Hueing>
class ColourDependantImplementation 
: public Viewer::Implementation,
  public DisplayHandler::ViewportHandler
{
    class ColouredImageAdapter;
    typedef HueingColorizer<Hueing> MyColorizer;
    typedef DiscretizedImage::ImageDiscretizer< 
        MyColorizer, ColouredImageAdapter>
        Discretizer;
    typedef BinnedLocalizations<Discretizer> Accumulator;

    static void copy_color( dStorm::Color& c, 
                            const typename MyColorizer::Pixel& p )
    {
        c.r = p.r;
        c.g = p.g;
        c.b = p.b;
    }

    class ColouredImageAdapter 
        : public DiscretizedImage::Listener<typename MyColorizer::Pixel>
    {
        Discretizer& discretizer;
        typedef std::vector< bool > PixelSet;
        PixelSet ps;
        int ps_step, ps_bits_set;
        std::auto_ptr<DisplayHandler::ImageHandle>
            image_handle;
        std::string desc;
        bool do_show_output;
        DisplayHandler::ViewportHandler& vph;

      public:
        ColouredImageAdapter( 
            Discretizer& disc, 
            const Viewer::Config& config,
            DisplayHandler::ViewportHandler& vph 
        ) : discretizer(disc), ps_bits_set(0), desc(config.getDesc()),
              do_show_output( config.showOutput() ),
              vph(vph)
            {}
        ~ColouredImageAdapter() {
            if ( image_handle.get() != NULL )
                close_window();
        }
        void setSize(int width, int height) { 
            if ( ! do_show_output ) return;

            ps.resize( width * height, false );
            ps_step = width;

            ChangeEvent::ResizeChange r;
            r.width = width;
            r.height = height;
            r.key_size = MyColorizer::BrightnessDepth;
            copy_color(r.background, discretizer.get_background());

            if ( ! image_handle.get() ) {
                set_image_handle(  
                    DisplayHandler::getSingleton()
                        .makeImageWindow(desc, vph, r));
            } else {
                image_handle->resize() = r;
                image_handle->commit_changes( 1 );
            }
        }
        void pixelChanged(int x, int y) {
            std::vector<bool>::reference& is_on = ps[ y * ps_step + x ];
            if ( ! is_on ) {
                ChangeEvent::PixelChange& p = 
                    image_handle->changed_pixel();
                p.x = x;
                p.y = y;
                /* The color field will be set when the clean handler
                 * runs. */
                ps_bits_set++;
            }
        }
        void clean() {
            if ( image_handle.get() == NULL ) return;
            typedef DisplayHandler::ImageHandle::PixelQueue PixQ;
            const PixQ::const_iterator end 
                = image_handle->get_pixel_queue().end();
            for ( PixQ::const_iterator i = 
                image_handle->get_pixel_queue().begin(); i != end; i++ )
            {
                copy_color(i->color, discretizer.get_pixel(i->x, i->y));
                ps[ i->y * ps_step + i->x] = false;
            }
            ps_bits_set = 0;
        }
        void clear() {
            if ( image_handle.get() == NULL ) return;
            ChangeEvent::ClearChange &c = image_handle->clear();
            copy_color(c.background, discretizer.get_background());
            image_handle->commit_changes( 1 );
            ps.clear();
            ps_bits_set = 0;
        }
        void notice_key_change( int index, 
            typename MyColorizer::Pixel pixel, float value )
        {
            if ( image_handle.get() == NULL ) return;
            ChangeEvent::KeyChange& k = image_handle->set_key();
            k.index = index;
            copy_color(k.color, pixel);
            k.value = value;
            image_handle->commit_changes( 1 );
        }

        void set_image_handle(
            std::auto_ptr<DisplayHandler::ImageHandle> handle)
        {
            image_handle = handle;
            clear();
        }
        void close_window() {
            if ( image_handle.get() != NULL ) {
                DisplayHandler::getSingleton()
                    .returnImageWindow( image_handle, true );
            }
        }
        void detach_window() {
            if ( image_handle.get() != NULL ) {
                DisplayHandler::getSingleton()
                    .returnImageWindow( image_handle, false );
            }
        }
        std::auto_ptr<DisplayHandler::ImageHandle> release_window() {
            do_show_output = false;
            return image_handle;
        }
                
    };

    void clean() {
        image.clean(); 
    }

    /** Binned image with all localizations in localizationsStore.*/
    Accumulator image;
    MyColorizer colorizer;
    /** Discretized version of \c image. */
    Discretizer discretization;
    ColouredImageAdapter cia;

  public:
    ColourDependantImplementation(const Viewer::Config& config)
        : /* Careful: Forward reference. This code works because (a) getMutex() is not virtual and (b) we simply need a reference. */
          DisplayHandler::ViewportHandler( image.getMutex() ),
          image( config.res_enh(), 1 ),
          colorizer(config),
          discretization( 4096, 1, 
                config.histogramPower(), image(),
                colorizer),
          cia( discretization, config, *this )
    {
        image.setListener(&discretization);
        discretization.setListener(&cia);
    }

    ~ColourDependantImplementation() {}

    dStorm::Output& getForwardOutput() { return image; }
    virtual std::auto_ptr< cimg_library::CImg<unsigned char> >
        full_size_image() 
    { 
        image.clean();
        return discretization.full_image();
    }

    virtual void set_histogram_power(float power) 
        { discretization.setHistogramPower( power ); }
    virtual void set_resolution_enhancement(float re) 
        { image.set_resolution_enhancement( re ); }
    virtual std::auto_ptr<DisplayHandler::ImageHandle>
        detach_from_window() { return cia.release_window(); }
    virtual void wait_for_completion() { cia.detach_window(); }
};

Viewer::_Config::_Config()
: simparm::Object("Image", "Image display"),
  showOutput("ShowOutput", "Display dSTORM result image"),
  outputFile("ToFile", "Save image to"),
  res_enh("ResEnhance", "Resolution Enhancement", 10),
  refreshCycle("ImageRefreshCycle", "Refresh image every x ms:", 100),
  histogramPower("HistogramPower", "Extent of histogram normalization",
                 0.3),
  zoom("ZoomLevel", "Zoom level", 0),
  colourScheme("ColourScheme", "Colour palette for display"),
  hue("Hue", "Select color hue", 0),
  saturation("Saturation", "Select saturation", 1),
  invert("InvertColours", "Invert colours", false),
  close_on_completion("CloseOnCompletion", 
                      "Close display on job completion")
{
    PROGRESS("Building Viewer Config");

    outputFile.helpID = HELP_Viewer_ToFile;
    outputFile.setUserLevel(simparm::Entry::Beginner);
    outputFile.default_extension = ".jpg";
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

    zoom.setMin(-10);
    zoom.setMax(10);
    zoom.setUserLevel(simparm::Entry::Expert);

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

    PROGRESS("Built Viewer Config");
}

void Viewer::_Config::registerNamedEntries() {
   register_entry(&outputFile);
   register_entry(&showOutput);
   register_entry(&res_enh);
   register_entry(&histogramPower);
   register_entry(&zoom);
   register_entry(&colourScheme);
   register_entry(&invert);
   register_entry(&hue);
   register_entry(&saturation);
   register_entry(&close_on_completion);
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
: Object("Display", "Display status"),
  implementation( 
    make_binned_locs< ColourSchemes::LastColourModel >( config ) ),
  forwardOutput( implementation->getForwardOutput() ),
  zi(1+max<int>(0,config.zoom())), 
  zo(1+max<int>(0,-config.zoom())),
  windowWidth(0), windowHeight(0),
  lastWW(0), lastWH(0),
  runViewer( config.showOutput() ),
  runningViewer(false),
  terminateViewer(false),
  needsResizing(false),
  isEmpty(true),
  close_display_immediately( config.close_on_completion() ),
  cyclelength( config.refreshCycle() / waitTime ),
  tifFile( config.outputFile ),
  resolutionEnhancement( config.res_enh ),
  histogramPower( config.histogramPower ),
  zoom( config.zoom ),
  save("SaveImage", "Save image"),
  quit("Quit", "Close viewing window")
{
    if (cimg_lock == NULL) cimg_lock = new Mutex();

    resolutionEnhancement.helpID = HELP_Viewer_Status_ResEnh;
    histogramPower.helpID = HELP_Viewer_Status_Power;
    zoom.helpID = HELP_Viewer_Status_Zoom;
    tifFile.helpID = HELP_Viewer_Status_ToFile;
    save.helpID = HELP_Viewer_Status_Save;

    /* With the values provided in config, meaningful defaults can
     * be set in the following config entries. */
    histogramPower.setUserLevel(simparm::Entry::Beginner);
    zoom.setUserLevel(simparm::Entry::Beginner);
    save.setUserLevel(simparm::Entry::Beginner);

    push_back( resolutionEnhancement );
    push_back( histogramPower );
    push_back( zoom );
    push_back( tifFile );
    push_back( save );
    // push_back( quit ); /* quit is pushed back in the subthread */

    receive_changes_from( save.value );
    receive_changes_from( quit.value );
    receive_changes_from( zoom.value );
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
    STATUS("Destructing Viewer");
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
    AdditionalData data = NoData;

    MutexLock lock(structureMutex);
    data = AdditionalData(data | forwardOutput.announceStormSize(a));

    return data;
}

void Viewer::propagate_signal(ProgressSignal s) {
    {
        MutexLock lock(structureMutex);
        forwardOutput.propagate_signal(s);
    }

    if ( s == Engine_is_restarted || s == Engine_run_failed )
        isEmpty = true;
    else if (s == Engine_run_succeeded && tifFile) {
        writeToFile(tifFile());
    } else if ( s == Prepare_destruction ) {
        terminateViewer = close_display_immediately 
                          || simparm::Node::isActive();
        if ( ! terminateViewer )
            implementation->wait_for_completion();
    }
}

void Viewer::operator()(Node& src, Node::Callback::Cause cause,
                        Node *) {
    if (cause != ValueChanged) return;
    if (&src == &save.value && save.triggered()) {
        /* Save image */
        save.untrigger();
        if ( tifFile ) {
            writeToFile( tifFile() );
        }
    } else if (&src == &quit.value && quit.triggered()) {
        /* Close viewer */
        quit.untrigger();
        terminateViewer = true;
    } else if (&src == &histogramPower.value) {
        MutexLock lock(structureMutex);
        /* Change histogram power */
        implementation->set_histogram_power(histogramPower());
    } else if (&src == &resolutionEnhancement.value) {
        MutexLock lock(structureMutex);
        /* Change resolution enhancement in viewer */
        implementation->
            set_resolution_enhancement( resolutionEnhancement() );
    } 
}

void Viewer::writeToFile(const string &name) {
    MutexLock lock(structureMutex);

    const char * const filename = name.c_str();

    /* If we are running in multi-threaded mode here, we must avoid
        * an exception since mingw is inherently troublesome with 
        * exceptions and MT. Therefore, first try to open the file
        * manually. */
    {
        ofstream tester( filename, ios_base::out );
        if ( ! tester ) {
            std::cerr << "Unable to write to file " << filename << "\n";
            return;
        }
    }

    std::auto_ptr< CImg<unsigned char> >
        normIm = implementation->full_size_image();

    try {
        /* Do not show CImg errors in windows. */
        cimg::exception_mode() = 0U;
        normIm->save_magick(filename);
    } catch (const cimg_library::CImgException& e) {
        cerr << "Unable to save image: " << e.message << endl;
        /* If an exception was thrown, a bug in the CImg memory
         * handling in save_cimg might have been triggered, making
         * destruction of normIm.data unsafe. Accept memory leak
         * in this case. */
        normIm.release();
    }
}

}
