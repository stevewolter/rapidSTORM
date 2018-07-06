#ifndef DSTORM_DISPLAY_MANAGER_H
#define DSTORM_DISPLAY_MANAGER_H

#include <memory>
#include <bitset>
#include "display/DataSource.h"
#include <boost/utility.hpp>
#include <boost/function.hpp>
#include "simparm/NodeHandle.h"

namespace dStorm {
namespace display {

class DataSource;
class ResizeChange;

struct SaveRequest {
    typedef boost::function<void (Change&)> ImageManipulator;
    std::string filename;
    boost::units::quantity< boost::units::si::length > scale_bar;
    ImageManipulator manipulator;

    SaveRequest();
};

/** WindowFlags summarize boolean flags for window
    *  behaviour. */
struct WindowFlags : public std::bitset<3> {
    /** If this flag is set, the window created by
        *  register_data_source is closed as soon as
        *  the WindowHandle is destroyed. Unset with
        *  leave_window_open_on_unregister.
        *  @param set Set the flag if this variable is
        *             true, unset otherwise. */
    WindowFlags& close_window_on_unregister
        (bool set = true)
        { this->set(0, set); return *this; }
    /** This method unsets the
        *   close_window_on_unregister flag. */
    WindowFlags& leave_window_open_on_unregister()
        { reset(0); return *this; }
    bool get_close_window_on_unregister() const
        { return (*this)[0]; }
    /** If this flag is set, the image handle
        *  destructor delays execution until the
        *  data window is closed by the user or by the
        *  close_window_on_unregister flag. If this
        *  flag is not set (default), leave the window
        *  open. This flag is reset by the 
        *  detach_from_window method.
        *
        *  @param set Set the flag if this variable is
        *             true, unset otherwise. */
    WindowFlags& wait_for_user_closing_window(bool set = true)
        { this->set(1, set); return *this; }
    /** This method unsets the wait_for_user_closing_window flag. */
    WindowFlags& detach_from_window()
        { this->reset(1); return *this; }
    /** If this flag is set to true, the data source's
        *  notice_drawn_rectangle method will be called when a rectangle
        *  drawing is completed. When not set (the default), the 
        *  rectangle will be taken as region to zoom to. */
    WindowFlags& notice_drawn_rectangle(bool set = true)
        { this->set(2, set); return *this; }
    /** This method unsets the notice_drawn_rectangle flag. */
    WindowFlags& zoom_on_drawn_rectangle()
        { this->reset(2); return *this; }
    bool get_notice_drawn_rectangle() const { return (*this)[2]; }
};

/** A WindowHandle is the representation of a data
    *  window. As long as a WindowHandle object lives
    *  for the DataSource that was provided in the
    *  register_data_source call, the DataSource object
    *  is considered valid and callable. */
class WindowHandle {
  protected:
    WindowHandle() {}
    WindowHandle(const WindowHandle&);
    WindowHandle& operator=(const WindowHandle&);
  public:
    WindowFlags flags;
    virtual ~WindowHandle() {}

    virtual void store_current_display( SaveRequest ) = 0;
};

/** The WindowProperties class provides the data
    *  necessary for creation of a data window by the
    *  register_data_source method. */
struct WindowProperties {
    /** Displayed name of the window. */
    std::string name;
    /** Flags used to modify window behaviour. Will
        *  be copied into the WindowHandle object. */
    WindowFlags flags;
    /** Initial size of the image data area. */
    ResizeChange initial_size;
};

}
}

#endif
