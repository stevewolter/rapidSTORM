#ifndef DSTORM_DISPLAY_WXMANAGER_H
#define DSTORM_DISPLAY_WXMANAGER_H

#include <map>
#include <queue>
#include <dStorm/helpers/thread.h>
#include "dStorm/helpers/DisplayManager.h"

namespace dStorm {
namespace Display {

class Window;

/** Implementation of the Manager class for the wxwidgets
 *  toolkit. This class forks a thread that runs the
 *  wxWidgets main loop and continues to run until
 *  close() is called. */
class wxManager : private ost::Thread, public Manager {
  public:
    class WindowHandle;
  private:
    //friend class Window;
    friend class WindowHandle;
    int open_handles;
    void increase_handle_count();
    void decrease_handle_count();

    ost::Mutex mutex;
    ost::Condition closed_all_handles;
     
    std::queue<ost::Runnable*> run_queue;

    bool was_started, may_close, toolkit_available;

    class Creator;
    class Disassociator;
    class Closer;
    class IdleCall;
    std::auto_ptr<IdleCall> idle_call;

    void exec(ost::Runnable* runnable);
    void run_in_GUI_thread( ost::Runnable* code );

    void run() throw();

  public:
    wxManager();
    /** Close all remaining windows and shut down the
     *  Manager. Warning: The manager should only be shut
     *  down once during the whole program execution. */
    ~wxManager();
    simparm::Node* getConfig() { return NULL; }

    std::auto_ptr<Manager::WindowHandle>
    register_data_source(
        const WindowProperties& properties,
        DataSource& handler
    );

    static void disassociate_window
        ( Window *window, WindowHandle* handle );

    void exec_waiting_runnables();

    void store_image(
        std::string filename,
        const Change& image );
};

}
}

#endif
