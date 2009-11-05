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
class wxManager : public ost::Thread, public Manager {
  public:
    class WindowHandle;
  private:
    friend class Window;

    ost::Mutex mutex;
     
    std::queue<ost::Runnable*> run_queue;

    class Creator;
    class Disassociator;

    wxManager();

    void exec(ost::Runnable* runnable);

  public:
    static wxManager& getSingleton();
    void run() throw();
    void close() throw();

    std::auto_ptr<Manager::WindowHandle>
    register_data_source(
        const WindowProperties& properties,
        DataSource& handler
    );

    static void disassociate_window
        ( Window *window, WindowHandle* handle );

    void run_in_GUI_thread( ost::Runnable* code );
    void exec_waiting_runnables();
};

}
}

#endif
