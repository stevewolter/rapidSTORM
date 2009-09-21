#define cimg_use_magick
#include <stdint.h>
#include "transmissions/Viewer.h"
#include "transmissions/ViewerConfig.h"
#include <limits>
#include <cassert>
#include "engine/Image.h"
#include <CImg.h>
#include <fstream>
#include "help_context.h"

#include <dStorm/transmissions/BinnedLocalizations.h>
#include <dStorm/transmissions/BinnedLocalizations_impl.h>
#include <dStorm/transmissions/HighDepthImage.h>
#include <dStorm/transmissions/HighDepthImage_impl.h>
#include <dStorm/transmissions/Histogram.h>
#include <dStorm/transmissions/Histogram_impl.h>
#include <dStorm/transmissions/ViewportImage.h>
#include <dStorm/transmissions/ColourDisplay.h>
#include <dStorm/transmissions/ColourDisplay_impl.h>

#ifndef UINT8_MAX
#define UINT8_MAX std::numeric_limits<uint8_t>::max()
#endif

using namespace std;
using namespace cimg_library;
using namespace ost;

static Mutex *cimg_lock = NULL;

namespace dStorm {

/** Time to wait for user input in milliseconds. */
const int waitTime = 10;

struct Viewer::Implementation
{
    virtual ~Implementation() {}

    virtual dStorm::Output& getForwardOutput() = 0;
    virtual void setViewport(int x0, int x1, int y0, int y1) = 0;
    virtual void shift( float x, float y ) = 0;
    virtual void cleanImage() = 0;

    virtual bool need_redisplay() = 0;
    virtual const cimg_library::CImg<unsigned char>& get_image()=0;
    virtual std::auto_ptr< cimg_library::CImg<unsigned char> >
        full_size_image() = 0;

    virtual void set_histogram_power(float power) = 0;
    virtual void set_resolution_enhancement(float re) = 0;

    virtual int getFullWidth() = 0;
    virtual int getFullHeight() = 0;
    virtual int getViewportWidth() = 0;
    virtual int getViewportHeight() = 0;
    virtual int leftBorder() = 0;
    virtual int topBorder() = 0;
};

template <int Hueing>
class ColourDependantImplementation 
: public Viewer::Implementation 
{
    typedef ColouredImage<Hueing> Image;
    typedef ViewportImage<DummyDiscretizationListener, Image> Viewport;
    typedef NormalizedHistogram<DummyDiscretizationListener, Viewport>
            Histogram;
    typedef HighDepthImage<Image, Histogram> Discretization;
    typedef BinnedLocalizations<Discretization> Accumulator;

    /** Binned image with all localizations in localizationsStore.*/
    Accumulator image;
    /** Discretized version of \c image. */
    Discretization discretization;
    /** Histogram and histogram-normalized transition function. */
    Histogram histogram;
    /** Viewport to histogram-equalized version of \c image */
    Viewport viewport;
    /** Colorized image displaying the brightness of \c result with
     *  colours choosen by hueing strategy. */
    Image result;

  public:
    ColourDependantImplementation(const Viewer::Config& config)
        : image( config.res_enh(), 1 ),
          discretization( 4096, 1 ),
          histogram( 4096, Image::DesiredDepth ),
          viewport(),
          result(config)
    {
        /* Initialize the update chain. For a normal update, the
        * Viewer will delegate the upkeep of \c result to \c discretization
        * and of \c discretization to \c image. */
        image.setListener(&discretization);
        discretization.setListener(&result);
        discretization.setListener(&histogram);
        histogram.setNormalizedListener(&viewport);
        viewport.setViewportListener(&result);

        histogram.setHistogramPower(config.histogramPower());
    }

    ~ColourDependantImplementation() { }

    dStorm::Output& getForwardOutput() { return image; }
    virtual void setViewport(int x0, int x1, int y0, int y1) 
        { viewport.setViewport(x0, x1, y0, y1); }
    virtual void shift( float x, float y ) 
        { viewport.shift( x, y); }

    virtual bool need_redisplay() 
        { return result.need_redisplay(); }
    virtual const cimg_library::CImg<unsigned char>& get_image()
        { return result.get_image(); }
    virtual std::auto_ptr< cimg_library::CImg<unsigned char> >
        full_size_image() 
    { 
        image.clean();
        return result.colour( viewport.full_size_image() );
    }

    void cleanImage() { 
        image.clean(); 
    }

    virtual void set_histogram_power(float power) 
        { histogram.setHistogramPower( power ); }
    virtual void set_resolution_enhancement(float re) 
        { image.set_resolution_enhancement( re ); }

    virtual int getFullWidth() { return image.width(); }
    virtual int getFullHeight() { return image.height(); }
    virtual int getViewportWidth() { return viewport.width(); }
    virtual int getViewportHeight() { return viewport.height(); }
    virtual int leftBorder() { return viewport.leftBorder(); }
    virtual int topBorder() { return viewport.topBorder(); }
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
  ost::Thread("Display"),
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
        string name = config.getDesc();
        int initial_width = 4*CImgDisplay::screen_dimx()/5,
            initial_height = 4*CImgDisplay::screen_dimy()/5;
        viewport.reset(new CImgDisplay(
            initial_width, initial_height, name.c_str(), 0));
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

    /** Set defaults for the viewport size */
    if (viewport.get()) {
        int cww = std::min<int>(viewport->window_width, 
                           4*CImgDisplay::screen_dimx()/5);
        int cwh = std::min<int>(viewport->window_height, 
                           4*CImgDisplay::screen_dimy()/5);
        int imw = implementation->getFullWidth(),
            imh = implementation->getFullHeight();

        int zoom_out = max(1, 
            max<int>( ceil(imw / cww), ceil( imh / cwh )));
        implementation->setViewport( 0, imw/zoom_out, 0, imh/zoom_out );
        zoom = - (zoom_out - 1);
    } else {
        implementation->setViewport( 0, 1, 0, 1 );
    }
    if ( runViewer ) {
        runningViewer = true;
        start();
    }

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
        join();
    }
}

void Viewer::run() throw() {
  try {
    /* This method is run in the event handling and displaying subthread. */
    PROGRESS("Starting display");
    /* Don't need quit push_back( quit ); */

    int cycles = 0;
    while ( !terminateViewer && ! viewport->is_closed ) {
        bool gotEvent = true;
        if ( (viewport->window_width/zi != windowWidth/zi &&
              lastWW != viewport->window_width) || 
             (viewport->window_height/zi != windowHeight/zi &&
              lastWH != viewport->window_height) ) 
        {
            /* Change in the viewport or the zoom level */
            needsResizing = resizeViewportToWindow();
        } else if (viewport->is_key(cimg::keyARROWRIGHT, true)) {
            MutexLock structureLock(structureMutex);
            PROGRESS("Shifting viewport right");
            implementation->shift( 0.5, 0);
        } else if (viewport->is_key(cimg::keyARROWLEFT, true)) {
            MutexLock structureLock(structureMutex);
            PROGRESS("Shifting viewport left");
            implementation->shift(-0.5, 0);
        } else if (viewport->is_key(cimg::keyARROWDOWN, true)) {
            MutexLock structureLock(structureMutex);
            PROGRESS("Shifting viewport down");
            implementation->shift(0,  0.5);
        } else if (viewport->is_key(cimg::keyARROWUP, true)) {
            MutexLock structureLock(structureMutex);
            PROGRESS("Shifting viewport up");
            implementation->shift(0, -0.5);
        } else if (viewport->is_key(cimg::keyI, true)) {
            /* Zoom-in action */
            zoom = zoom()+1;
            int zL = zoom();
            zi = zo = 1;
            if (zL > 0) zi = 1+zL; else zo = 1-zL;
            needsResizing = resizeViewportToWindow();
        } else if (viewport->is_key(cimg::keyO, true)) {
            /* Zoom-out action */
            zoom = zoom()-1;
            int zL = zoom();
            zi = zo = 1;
            if (zL > 0) zi = 1+zL; else zo = 1-zL;
            needsResizing = resizeViewportToWindow();
        } else if (viewport->is_key(cimg::keyQ, true)) {
            PROGRESS("Closing viewport");
            MutexLock lock(*cimg_lock);
            viewport->close();
        } else {
            PROGRESS("Waiting for event");
            MutexLock lock(*cimg_lock);
            cimg::wait(waitTime);
            cycles++;
            gotEvent = false;
        }
        lastWW = viewport->window_width;
        lastWH = viewport->window_height;

        if ( gotEvent || cycles % cyclelength == 0 ) {
            MutexLock structureLock(structureMutex);
            implementation->cleanImage();

            if (windowWidth != viewport->width ||
                windowHeight != viewport->height) 
            {
                LOCKING("Acquiring cimg_lock");
                MutexLock lock(*cimg_lock);
                PROGRESS("Resizing result");
                viewport->resize(windowWidth, windowHeight, false);
                PROGRESS("Resized result");
            } 
            if (gotEvent || implementation->need_redisplay()) {
                LOCKING("Acquiring cimg_lock");
                MutexLock lock(*cimg_lock);
                PROGRESS("Redisplaying image");
                const CImg<unsigned char>& c = implementation->get_image();
                PROGRESS("Sending image " << c.width << " " << c.height 
                            << " to display");
                viewport->display(c);
                PROGRESS("Displayed image");
            }
        }
    }

    viewport->close();
    erase( quit );
  } catch (const std::exception& e) {
    std::cerr << "Error in concurrent image display: "
              << e.what() << endl;
  } catch (...) {
    std::cerr << "Unknown error in concurrent image display. "
              << endl;
  }
  runningViewer = false;
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
    } else if (&src == &zoom.value) {
        /* Change zoom level */
        int zL = zoom();
        zi = zo = 1;
        if (zL > 0) zi = 1+zL; else zo = 1-zL;
        if (viewport.get())
            needsResizing = resizeViewportToWindow();
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
        //cimg::exception_mode() = 0U;
        /* Do not show CImg errors in windows. */
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

bool Viewer::resizeViewportToWindow() {
    MutexLock structureLock(structureMutex);
    if ( viewport->window_width == 0 || viewport->window_height == 0 )
        return false;

    PROGRESS("Resizing viewport to window size");

    /* The desired viewport width and height */
    int vw = min<int>(implementation->getFullWidth(),
                      zo*viewport->window_width/zi),
        vh = min<int>(implementation->getFullHeight(),
                      zo*viewport->window_height/zi);

    /* Difference to current window */
    int dw = implementation->getViewportWidth()-vw,
        dh = implementation->getViewportHeight()-vh;

    /* Both result in a shift */
    int dx = dw/2, dy = dh/2;

    /* Try to align the centers of new and old viewports, but
     * move if too close to border. */
    int x0 = implementation->leftBorder()+dx, 
        y0 = implementation->topBorder()+dy;
    x0 = max(0, min(x0, implementation->getFullWidth()-vw));
    y0 = max(0, min(y0, implementation->getFullHeight()-vh));
    int rw = min(implementation->getFullWidth(), vw),
        rh = min(implementation->getFullHeight(), vh);

    implementation->setViewport(x0, x0+rw-1, y0, y0+rh-1);
    windowWidth = zi*implementation->getViewportWidth()/zo;
    windowHeight = zi*implementation->getViewportHeight()/zo;
    PROGRESS("New viewport is " << x0 << "-" << x0+rw-1 << " " << y0 << "-" << y0+rh-1);
    return (vw != rw || vh != rh);
}

}
