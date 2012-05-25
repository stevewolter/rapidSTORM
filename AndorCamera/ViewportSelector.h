#ifndef ANDORDIRECT_VIEWPORTSELECTOR_H
#define ANDORDIRECT_VIEWPORTSELECTOR_H

#include <dStorm/stack_realign.h>
#include <boost/thread/mutex.hpp>
#include <dStorm/display/Manager.h>
#include <simparm/Entry.h>
#include <simparm/FileEntry.h>
#include <simparm/TriggerEntry.h>
#include <simparm/Group.h>
#include <boost/optional.hpp>
#include <map>
#include "AndorDirect.h"

#include <dStorm/engine/InputTraits.h>
#include <boost/smart_ptr/scoped_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <dStorm/traits/image_resolution.h>

namespace dStorm {
namespace AndorCamera {

/** The Display provides a window in which Entry 
    *  elements defining the acquisition rectangle can be displayed
    *  and configured interactively. */
class Display : public display::DataSource
{
  private:
    struct FetchHandler;

    boost::scoped_ptr<CameraConnection> cam;
    Method& config;

    simparm::Group               name_object;
    simparm::StringEntry       status;
    simparm::TriggerEntry      stopAim;
    simparm::TriggerEntry      pause;
    simparm::FileEntry         imageFile;
    simparm::TriggerEntry      save;

    simparm::NodeHandle current_ui;
    simparm::BaseAttribute::ConnectionStore listening[3];

    void do_pause();
    void do_save();
    void do_stop();

    /** Reference to the config element to be configured. */
    bool aimed;

    boost::mutex mutex;
    /** Flag is set to true when the display should be paused. */
    bool paused;

    boost::optional<int> camBorders[4];

    /** Buffer image for acquisition. Made class member to allow 
        *  saving to file. */
    std::auto_ptr<display::Change> change;
    std::auto_ptr<display::Manager::WindowHandle> handle;

    CamTraits traits;
    /** Currently used normalization boundaries. Will be set for each new
     *  image when \c lock_normalization is not set. */
    CamImage::PixelPair normalization_factor;
    /** If set to true, \c normalization_factor is fixed at the current
     *  level. */
    bool lock_normalization, redeclare_key;
    boost::optional< boost::units::quantity<boost::units::camera::intensity> >
        lower_user_limit, upper_user_limit;

    /** Saved data of the last camera image to enable saving. */
    dStorm::display::Image last_image;
    image::MetaInfo<2>::Resolutions resolution;
    boost::thread image_acquirer;

    /** Subthread for image acquisition. */
    DSTORM_REALIGN_STACK virtual void run() throw();
    /** Method implementing data source interface. */
    virtual std::auto_ptr<display::Change> get_changes();
    /** Method implementing data source interface. */
    virtual void notice_closed_data_window();
    /** Method implementing data source interface. */
    virtual void notice_drawn_rectangle(int, int, int, int);
    /** Method implementing data source interface. */
    void notice_user_key_limits(int, bool, std::string);

    void acquire();
    void configure_camera();
    void initialize_display();
    void draw_image(const CamImage& data);

    display::ResizeChange getSize() const;

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
    Display ( std::auto_ptr<CameraConnection>, Mode, Method& );
    /** Destructor, will join the subthread and close the display
        *  window. */
    virtual ~Display();

    void terminate();
    void resolution_changed( image::MetaInfo<2>::Resolutions );
    void basename_changed( const std::string& basename );
    void attach_ui( simparm::NodeHandle at );
};

}
}

#endif
