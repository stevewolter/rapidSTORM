#ifndef DSTORM_VIEWER_H
#define DSTORM_VIEWER_H

#include <dStorm/engine/Localizations.h>
#include <simparm/Entry.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/NumericEntry.hh>
#include <simparm/FileEntry.hh>
#include <simparm/Structure.hh>
#include <cc++/thread.h>
#include <outputs/LocalizationList.h>
#include <dStorm/Output.h>
#include <dStorm/OutputBuilder.h>
#include <dStorm/FileOutputBuilder.h>
#include <Eigen/Core>

namespace dStorm {

/** The Viewer class collects fits into a BinnedLocalizations
*  image, normalizes the resulting image and shows a part
*  of that image in a window. */
class Viewer : public dStorm::Output, public simparm::Object,
                public simparm::Node::Callback
{
  private:
    class _Config;
    class Implementation;

  public:
    typedef simparm::Structure<_Config> Config;
    typedef dStorm::FileOutputBuilder<Viewer> Source;

    /** Constructor will not display image; this is deferred
        *  until announceStormSize(). */
    Viewer(const Config& config);
    Viewer(const Viewer&);
    virtual ~Viewer();
    Viewer* clone() const 
        { throw std::runtime_error("No Viewer::clone()"); }

    AdditionalData announceStormSize(const Announcement &a);
    Result receiveLocalizations(const EngineResult&);
    void propagate_signal(ProgressSignal s);

  protected:
    /** Configuration event received. */
    void operator()(Node&, Cause, Node *);

    /** Write the current image into a file. The whole
        *  image is written, regardless of the settings
        *  for viewport or zoom. */
    void writeToFile(const std::string& name);

  private:
    std::auto_ptr< Implementation > implementation;
    dStorm::Output& forwardOutput;

    /** Mutex protecting \c implementation.*/
    ost::Mutex structureMutex;
    /** Factor to zoom the image in or out. By default, both
    *  are 1; to zoom in, zi is raised, to zoom out, zo is
    *  raised. */
    int zi, zo;
    /** Desired width and height of the display window. */
    unsigned int windowWidth, windowHeight;
    /** The last encountered dimensions for \c viewport. */
    unsigned int lastWW, lastWH;

    /** Should the viewer be run? */
    bool runViewer;
    /** Is the subthread currently running? */
    bool runningViewer;
    /** Should the subthread be terminated? */
    bool terminateViewer;
    /** Should \c viewport be resized as soon as possible? */
    bool needsResizing;
    /** Did this viewer get any results? */
    bool isEmpty;
    /** Shall we immediately close our display window on job completion? */
    bool close_display_immediately;
        
    /** Minimum time between refreshes of the displayed image. */
    int cyclelength;

    simparm::FileEntry tifFile;
    simparm::DoubleEntry resolutionEnhancement, histogramPower;
    simparm::LongEntry zoom;
    /** Config triggers for saving the currently displayed result
    *  image and for closing the display window. */
    simparm::TriggerEntry save, quit;

    /** Check all thresholds in the chain from \c image,
    *  \c discretization to \c result and recompute necessary
    *  results. */
    void cleanImage();
    /** Resize \c result to match, if possible, the window size
    *  of \c viewport.
    *
    *  @return true if \c viewport must be resized to match
    *          result.
    **/
    bool resizeViewportToWindow();

};

}

#endif
