#ifndef DSTORM_DISPLAY_WXMANAGER_H
#define DSTORM_DISPLAY_WXMANAGER_H

#include <dStorm/stack_realign.h>
#include <map>
#include <queue>
#include <dStorm/display/Manager.h>
#include <boost/thread/thread.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/function/function0.hpp>
#include <boost/ptr_container/ptr_list.hpp>

namespace dStorm {
namespace display {

class Window;

/** Implementation of the Manager class for the wxwidgets
 *  toolkit. This class forks a thread that runs the
 *  wxWidgets main loop and continues to run until
 *  close() is called. */
class wxManager {
  public:
    class WindowHandle;
  private:
    typedef boost::function0<void> Runnable;
    //friend class Window;
    friend class WindowHandle;
    int open_handles;
    void increase_handle_count();
    void decrease_handle_count();

    boost::recursive_mutex mutex;
    boost::condition closed_all_handles;
     
    boost::ptr_list<Runnable> run_queue;

    bool was_started, may_close, toolkit_available;
    boost::thread gui_thread;

    class Creator;
    class Disassociator;
    class Closer;
    class IdleCall;
    std::auto_ptr<IdleCall> idle_call;

    void exec(Runnable& runnable);
    void run_in_GUI_thread( std::auto_ptr<Runnable> code );
    template <typename Functor>
    void run_in_GUI_thread( const Functor& f ) {
        run_in_GUI_thread( std::auto_ptr<Runnable>( new Runnable(f) ) );
    }

    DSTORM_REALIGN_STACK void run() throw();

    wxManager();

  public:
    static wxManager& get_singleton_instance();

    /** Close all remaining windows and shut down the
     *  Manager. Warning: The manager should only be shut
     *  down once during the whole program execution. */
    ~wxManager();

    std::auto_ptr<display::WindowHandle>
    register_data_source(
        const WindowProperties& properties,
        DataSource& handler
    );

    static void disassociate_window
        ( Window *window, WindowHandle* handle );

    void exec_waiting_runnables();
};

}
}

#endif
