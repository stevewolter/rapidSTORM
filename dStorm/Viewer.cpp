#include <stdint.h>
#include "Viewer.h"
#include <limits>
#include <cassert>
#include <dStorm/Image.h>
#include <CImgBuffer/Image.h>

#ifndef UINT8_MAX
#define UINT8_MAX std::numeric_limits<uint8_t>::max()
#endif

using namespace std;
using namespace cimg_library;
using namespace ost;

static Mutex *cimg_lock = NULL;

namespace dStorm {

ViewerConfig::ViewerConfig()
: ConfigObject("ViewerConfig", "Image display options"),
  showOutput("ShowOutput", "Display dSTORM result image", false),
  tifFile("ImageOutput", "Save image to"),
  res_enh("ResEnhance", "Resolution Enhancement", 10),
  refreshCycle("ImageRefreshCycle", "Refresh image every x ms:", 100),
  amplitudeThreshold("ViewerAmplitudeThreshold", "Do not view localizations with "
                     "strength below", 0),
  histogramPower("HistogramPower", "Histogram normalization power", 2),
  zoom("ZoomLevel", "Zoom level", 0),
  colourScheme("ColourScheme", "Colour palette for display"),
  save("SaveImage", "Save image"),
  quit("Quit", "Close viewing window")
{
    PROGRESS("Building Viewer Config");

    tifFile.setUserLevel(ConfigEntry::Beginner);
    showOutput.setUserLevel(ConfigEntry::Beginner);
    res_enh.setHelp("The target image will have a resolution this many "
                    "times higher than the source image. For example, "
                    "a 128x128 image with resolution 10 will result in "
                    "a 1280x1280 sized image.");
    res_enh.setUserLevel(ConfigEntry::Intermediate);
    refreshCycle.setUserLevel(ConfigEntry::Intermediate);

    save.setUserLevel(ConfigEntry::Beginner);
    save.setViewable(false);
    save.setEditable(false);
    quit.setViewable(false);
    quit.setEditable(false);

    /* This level is reset in carStarted() */
    amplitudeThreshold.setUserLevel(ConfigEntry::Expert);

    histogramPower.setMin(1E-1);
    histogramPower.setMax(1E1);
    /* This level is reset in carStarted() */
    histogramPower.setUserLevel(ConfigEntry::Expert);

    zoom.setMin(-10);
    zoom.setMax(10);
    zoom.setUserLevel(ConfigEntry::Expert);

    colourScheme.addChoice("BlackWhite", "Black on white");
    colourScheme.addChoice("GreenBlack", "Green on black");

    PROGRESS("Built Viewer Config");
}

void ViewerConfig::registerEntries(ConfigSet &at) throw() {
   at.register_entry(&tifFile);
   at.register_entry(&showOutput);
   at.register_entry(&res_enh);
   at.register_entry(&amplitudeThreshold);
   at.register_entry(&histogramPower);
   at.register_entry(&zoom);
   at.register_entry(&colourScheme);
   at.register_entry(&save);
   at.register_entry(&quit);
}


Viewer::Viewer(ViewerConfig& config) throw()
: ConfigObject("Display", "Display status"),
  ost::Thread("Display"),
  image(config.res_enh(), 1),
  config(config),
  discretization(4096, 1.1),
  result( (config.colourScheme.getValue() == "BlackWhite") ? 1 : 3,
          (config.colourScheme.getValue() == "GreenBlack") ? 1 : 0,
          (config.colourScheme.getValue() == "BlackWhite") ),
  /* Preliminary values that are replaced in carStarted() */
  minStrength(1),
  maxStrength(2*minStrength),
  ampThres(1),
  zi(1+max<int>(0,config.zoom())), 
  zo(1+max<int>(0,-config.zoom())),
  windowWidth(0), windowHeight(0),
  runningViewer(false),
  terminateViewer(false),
  ampThresChanged(false),
  needsResizing(false)
{
    if (cimg_lock == NULL) cimg_lock = new Mutex();

    minStrength = 1000;
    maxStrength = minStrength * 2;
    ampThres = minStrength;

    /* With the values provided in config, meaningful defaults can
     * be set in the following config entries. In addition, actions
     * such as quit and save become sensible. */
    config.amplitudeThreshold.setMin(minStrength);
    config.amplitudeThreshold.setMax(maxStrength);
    config.amplitudeThreshold.setUserLevel(ConfigEntry::Beginner);
    config.histogramPower.setUserLevel(ConfigEntry::Beginner);
    config.zoom.setUserLevel(ConfigEntry::Beginner);
    config.save.setViewable(true);
    config.quit.setViewable(true);
    config.save.setEditable(true);
    config.quit.setEditable(true);
    config.register_entry(&config.amplitudeThreshold);
    config.register_entry(&config.histogramPower);
    config.register_entry(&config.zoom);
    config.register_entry(&config.tifFile);
    config.register_entry(&config.save);

    /* Initialize the update chain. For a normal update, the
     * Viewer will delegate the upkeep of \c result to \c discretization
     * and of \c discretization to \c image. */
    image.setListener(&discretization);
    discretization.setListener(&result.getHistogram());
    discretization.setListener(&result);

    receive_changes_from( config.save );
    receive_changes_from( config.quit );
    receive_changes_from( config.histogramPower );
    receive_changes_from( config.amplitudeThreshold );
    receive_changes_from( config.zoom );

    result.getHistogram().setHistogramPower(1/config.histogramPower());

    if (config.showOutput() )
        viewport.reset(new CImgDisplay(
            4*CImgDisplay::screen_dimx()/5, 
            4*CImgDisplay::screen_dimy()/5,
            config.desc().c_str(), 0));
}

Viewer::~Viewer() throw() {
    if (localizationsStore.hasResults() &&
                localizationsStore.getResults().size() == 0)
        terminateViewer = true;
    join();
    STATUS("Destructing Viewer");
}


Transmission::Result
Viewer::receiveLocalizations(const EngineResult& er) throw()
{
    Result r = KeepRunning;
    PROGRESS("Storing fits in list");
    {
        MutexLock stlock(locStoreMutex);
        localizationsStore.receiveLocalizations(er);
    }

    PROGRESS("Updating amplitude threshold");
    for (int i = 0; i < er.number; i++) {
        if (er.first[i].getStrength() > maxStrength) {
            maxStrength = er.first[i].getStrength();
            config.amplitudeThreshold.setMax(
                max<double>(2*minStrength, maxStrength/4));
        }
    }

    {
        LOCKING("Acquiring structure mutex");
        MutexLock lock(structureMutex);
        PROGRESS("Adding fits to bin image");
        r = image.receiveLocalizations(er);
        if (r == KeepRunning && !runningViewer && !config.tifFile)
            r = RemoveTransmission;
        LOCKING("Releasing structure mutex");
    }
    return r;
}

Transmission::AdditionalData Viewer::announceStormSize(int w, int h, int l) throw() {
    AdditionalData data = NoData;
    {
        MutexLock stlock(locStoreMutex);
        data = AdditionalData(data | localizationsStore.announceStormSize(w,h,l));
    }

    MutexLock lock(structureMutex);
    data = AdditionalData(data | image.announceStormSize(w,h,l));

    /** Set defaults for the viewport size */
    if (viewport.get()) {
        int ww = (viewport->window_width > 0) 
            ?  min<int>(w, viewport->window_width) : w;
        int wh = (viewport->window_height > 0) 
            ?  min<int>(w, viewport->window_height) : h;
        PROGRESS("Setting viewport to 0-" << ww -1 << " 0," << wh-1);
        result.setViewport( 0, ww-1, 0, wh-1 );
    } else {
        result.setViewport( 0, 1, 0, 1 );
    }
    if ( config.showOutput() ) {
        runningViewer = true;
        start();
    }

    return data;
}

void Viewer::deleteAllResults() throw() {
    {
        MutexLock stlock(locStoreMutex);
        localizationsStore.deleteAllResults();
    }

    MutexLock lock(structureMutex);
    image.deleteAllResults();
}

void Viewer::finish() throw() {
    {
        MutexLock stlock(locStoreMutex);
        localizationsStore.finish();
    }

    {
        MutexLock lock(structureMutex);
        image.finish();
    }

    if (config.tifFile) {
        writeToFile(config.tifFile());
    }
}


void Viewer::run() throw() {
    /* This method is run in the event handling and displaying subthread. */
    PROGRESS("Starting display");
    config.quit.setViewable(true);
    config.quit.setEditable(true);

    int waitTime = 10;
    int cycles = 0, cyclelength = config.refreshCycle() / waitTime;
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
            result.shift( 0.5, 0);
        } else if (viewport->is_key(cimg::keyARROWLEFT, true)) {
            MutexLock structureLock(structureMutex);
            PROGRESS("Shifting viewport left");
            result.shift(-0.5, 0);
        } else if (viewport->is_key(cimg::keyARROWDOWN, true)) {
            MutexLock structureLock(structureMutex);
            PROGRESS("Shifting viewport down");
            result.shift(0,  0.5);
        } else if (viewport->is_key(cimg::keyARROWUP, true)) {
            MutexLock structureLock(structureMutex);
            PROGRESS("Shifting viewport up");
            result.shift(0, -0.5);
        } else if (viewport->is_key(cimg::keyI, true)) {
            /* Zoom-in action */
            config.zoom = config.zoom()+1;
            int zL = config.zoom();
            zi = zo = 1;
            if (zL > 0) zi = 1+zL; else zo = 1-zL;
            needsResizing = resizeViewportToWindow();
        } else if (viewport->is_key(cimg::keyO, true)) {
            /* Zoom-out action */
            config.zoom = config.zoom()-1;
            int zL = config.zoom();
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
            cleanImage();

            MutexLock structureLock(structureMutex);
            if (windowWidth != viewport->width ||
                windowHeight != viewport->height) 
            {
                LOCKING("Acquiring cimg_lock");
                MutexLock lock(*cimg_lock);
                PROGRESS("Resizing result");
                viewport->resize(windowWidth, windowHeight, false);
                PROGRESS("Resized result");
            } 
            if (gotEvent || result.isDirty(20)) {
                LOCKING("Acquiring cimg_lock");
                MutexLock lock(*cimg_lock);
                PROGRESS("Redisplaying image");
                const CImg<unsigned char>& c = result.redisplay();
                PROGRESS("Sending image " << c.width << " " << c.height 
                            << " to display");
                viewport->display(c);
                PROGRESS("Displayed image");
            }
        }
    }

    config.quit.setViewable(false);
    config.quit.setEditable(false);
    runningViewer = false;
}

void Viewer::cleanImage() throw() {
    if (ampThresChanged) {
        /* These mutex locks must be acquired in this sequence to avoid
         * deadlocks. */
        MutexLock a(locStoreMutex);
        MutexLock b(structureMutex);

        /* The result chain is cleared and refilled here.
         * The clean() further below will take care of
         * recomputations that were necessary. */
        image.clear();
        image.setAmplitudeThreshold(ampThres);
        Image dummy(0,0,0);
        const Localizations& fitList = localizationsStore.getResults();
        EngineResult result;
        for (int i = 0; i < fitList.binNumber(); i++) {
            result.first = fitList.getBin(i);
            result.number = fitList.sizeOfBin(i);
            image.receiveLocalizations(result);
        }
        ampThresChanged = false;
    }

    LOCKING("Acquiring structure lock");
    MutexLock structureLock(structureMutex);
    PROGRESS("Cleaning structures");
    image.clean();
    PROGRESS("Cleaned structures");

    if (result.needsRecompute()) {
        PROGRESS("Recomputing result");
        result.recompute(discretization.getImage());
        PROGRESS("Recomputed result");
    }
}

void Viewer::operator()(ConfigNode& src, ConfigNode::Callback::Cause cause,
                        ConfigNode *) throw() {
    if (cause != ValueChanged) return;
    if (&src == &config.save && config.save.triggered()) {
        /* Save image */
        config.save.untrigger();
        if ( config.tifFile )
            writeToFile( config.tifFile() );
    } else if (&src == &config.quit && config.quit.triggered()) {
        /* Close viewer */
        config.quit.untrigger();
        terminateViewer = true;
    } else if (&src == &config.histogramPower) {
        /* Change histogram power */
        result.getHistogram().setHistogramPower(1/config.histogramPower());
    } else if (&src == &config.amplitudeThreshold) {
        /* Change amplitude threshold */
        if (config.amplitudeThreshold() != ampThres) {
            ampThresChanged = true;
            ampThres = config.amplitudeThreshold();
        }
    } else if (&src == &config.zoom) {
        /* Change zoom level */
        int zL = config.zoom();
        zi = zo = 1;
        if (zL > 0) zi = 1+zL; else zo = 1-zL;
        if (viewport.get())
            needsResizing = resizeViewportToWindow();
    }
}

void Viewer::writeToFile(const string &name) throw() {
    MutexLock lock(structureMutex);
    image.clean();
    int dimNum = result.getNumberOfDimensions(),
        filledDim = result.getFilledDimension();
    CImg<uint8_t> normIm(image.width(), image.height(), 1, dimNum);
    unsigned long sz = image.width() * image.height();
    uint8_t *base = normIm.ptr(0,0,0,filledDim);
    for (unsigned long i = 0; i < sz; i++)
        base[i] = result.getHistogram().convert
                        (discretization.getImage()[i]);

    try {
#if HAVE_LIBMAGICK__
        normIm.save(name.c_str());
#else
#if cimg_version >= 129
        normIm.save_imagemagick_external(name.c_str());
#else
        normIm.save_imagemagick(name.c_str());
#endif
#endif
    } catch (const CImgException &e) {
        cerr << "CImg: " << e.message << endl;
    }
}

bool Viewer::resizeViewportToWindow() throw() {
    MutexLock structureLock(structureMutex);

    PROGRESS("Resizing viewport to window size");

    /* The desired viewport width and height */
    int vw = min<int>(image.width(), zo*viewport->window_width/zi),
        vh = min<int>(image.height(), zo*viewport->window_height/zi);

    /* Difference to current window */
    int dw = result.width()-vw, dh = result.height()-vh;

    /* Both result in a shift */
    int dx = dw/2, dy = dh/2;

    /* Try to align the centers of new and old viewports, but
     * move if too close to border. */
    int x0 = result.leftBorder()+dx, y0 = result.topBorder()+dy;
    x0 = max(0, min(x0, image.width()-vw));
    y0 = max(0, min(y0, image.height()-vh));
    int rw = min(image.width(), vw),
        rh = min(image.height(), vh);

    result.setViewport(x0, x0+rw-1, y0, y0+rh-1);
    windowWidth = zi*result.width()/zo;
    windowHeight = zi*result.height()/zo;
    PROGRESS("New viewport is " << x0 << "-" << x0+rw-1 << " " << y0 << "-" << y0+rh-1);
    return (vw != rw || vh != rh);
}

}
