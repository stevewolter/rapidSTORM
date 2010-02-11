#ifndef ANDORDIRECT_VIEWPORTSELECTOR_H
#define ANDORDIRECT_VIEWPORTSELECTOR_H

#ifdef HAVE_LIBATMCD32D

#include <dStorm/helpers/thread.h>
#include <dStorm/helpers/DisplayManager.h>
#include <simparm/Entry.hh>
#include <simparm/NumericEntry.hh>
#include <simparm/FileEntry.hh>
#include <simparm/TriggerEntry.hh>
#include <simparm/Set.hh>
#include <map>
#include <CImg.h>
#include <AndorCamera/CameraReference.h>
#include "AndorDirect.h"

#include <dStorm/input/ImageTraits.h>

namespace AndorCamera {

class Acquisition;

namespace ViewportSelector {

typedef dStorm::AndorDirect::CamTraits::Resolution Resolution;

class Config;

/** The Display provides a window in which Entry 
    *  elements defining the acquisition rectangle can be displayed
    *  and configured interactively. */
class Display : public simparm::Set,
                private simparm::Node::Callback, 
                private ost::Thread,
                public dStorm::Display::DataSource
{
  private:
    AndorCamera::CameraReference cam;
    Config& config;

    simparm::Object            statusBox;
    simparm::TriggerEntry      stopAim;
    simparm::TriggerEntry      pause;
    simparm::FileEntry         imageFile;
    simparm::TriggerEntry      save;

    void operator()(const simparm::Event&);

    /** Reference to the config element to be configured. */
    ImageReadout* aimed;

    ost::Mutex mutex;
    /** Flag is set to true when the display should be paused. */
    bool paused;

    /** Buffer image for acquisition. Made class member to allow 
        *  saving to file. */
    std::auto_ptr<dStorm::Display::Change> change;
    std::auto_ptr<dStorm::Display::Manager::WindowHandle> handle;

    /** Width of displayed camera image. */
    int width, height /**< Height of displayed camera image */;
    /** Currently used normalization factor. Will be set for each new
     *  image when \c lock_normalization is not set. */
    float normalization_factor;
    /** If set to true, \c normalization_factor is fixed at the current
     *  level. */
    bool lock_normalization;

    Resolution resolution;

    /** Saved data of the last camera image to enable saving. */
    data_cpp::Vector<dStorm::Display::Color> last_image;

    /** Subthread for image acquisition. */
    virtual void run() throw();
    /** Method implementing data source interface. */
    virtual std::auto_ptr<dStorm::Display::Change> get_changes();
    /** Method implementing data source interface. */
    virtual void notice_closed_data_window();
    /** Method implementing data source interface. */
    virtual void notice_drawn_rectangle(int, int, int, int);

    void registerNamedEntries();
    void acquire();
    void configure_camera(AndorCamera::Acquisition&);
    void initialize_display();
    void draw_image(const dStorm::AndorDirect::CameraPixel *data);

    dStorm::Display::ResizeChange getSize() const;

    /** Undefined copy constructor to avoid implicit copy construction. */
    Display(const Display&);
    /** Undefined assignment to avoid implicit assignment. */
    Display& operator=(const Display&);

  public:
    enum Mode { SelectROI, ViewROI };

    /** Constructor and only public interface.
        *  This is a fire-and-forget class: The constructor starts a
        *  subthread that will open the acquisition and update the
        *  display window, and then return control. */
    Display ( const CameraReference&, Mode, Config&, Resolution );
    /** Destructor, will join the subthread and close the display
        *  window. */
    virtual ~Display();

    void terminate();
    void set_resolution( const Resolution& );
};

/** Configuration items for the viewport selection window that
    *  opens when the "aim" button is pressed. */
class Config 
: public simparm::Object, public simparm::Node::Callback 
{
    AndorCamera::CameraReference cam;
    Resolution resolution;

    void registerNamedEntries();

    void startAiming();
    void stopAiming();

  public:
    simparm::TriggerEntry select_ROI, view_ROI;

    Config(const AndorCamera::CameraReference& cam, 
           Resolution);
    Config(const Config &c);
    ~Config();
    Config* clone();

    Config& operator=(const Config&);

    void operator()(const simparm::Event&);

    void delete_active_selector();
    void set_resolution( const Resolution& );
    const Resolution& get_resolution() { return resolution; }

  private:
    ost::Mutex active_selector_mutex;
    ost::Condition active_selector_changed;
    std::auto_ptr<Display> active_selector;

    void set_entry_viewability();
    void make_display( Display::Mode mode );
};

}
}

#endif
#endif
