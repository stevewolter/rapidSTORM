#ifndef DSTORM_DISPLAY_MANAGER_H
#define DSTORM_DISPLAY_MANAGER_H

#include <memory>
#include <bitset>
#include <dStorm/helpers/DisplayDataSource.h>
#include <dStorm/helpers/thread.h>

namespace dStorm {
namespace Display {

class DataSource;
class ResizeChange;

/** The Manager class provides is the primary public
 *  interface to the dStorm::Display module. Modules
 *  wishing to show data in image windows should acquire
 *  an WindowHandle using the register_data_source call,
 *  providing a callback through the DataSource interface.
 */
class Manager {
  public:
    static Manager& getSingleton();

    /** WindowFlags summarize boolean flags for window
     *  behaviour. */
    struct WindowFlags : public std::bitset<1> {
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
        WindowFlags& wait_for_user_closing_window
            (bool set = true)
            { this->set(1, set); return *this; }
        /** This method unsets the
         *   wait_for_user_closing_window flag. */
        WindowFlags& detach_from_window()
            { this->reset(1); return *this; }
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
    };

    /** Open a window that displays the data in \c handler
     *  with the provided \c properties. The provided
     *  DataSource must be valid until the WindowHandle
     *  object returned by the function is deleted.
     **/
    virtual std::auto_ptr<WindowHandle>
        register_data_source
        (const WindowProperties& properties,
         DataSource& handler) = 0;
    /** Run a piece of code in the GUI event thread.
     *  This method is intended for users which are
     *  not satisfied with the \c register_data_source
     *  interface, but need to do GUI stuff. */
    virtual void run_in_GUI_thread( ost::Runnable* code )=0;
    /** Determines whether the display manager was started
     *  at all. Non-thread-safe method prone to race
     *  conditions. */
    static bool was_started();
    /** Close all remaining windows and shut down the
     *  Manager. Warning: The manager should only be shut
     *  down once during the whole program execution. */
    virtual void close() = 0;

  protected:
    Manager() {}
    Manager(const Manager&);
    virtual ~Manager() {}
    Manager& operator=(const Manager&);
};

}
}

#endif
