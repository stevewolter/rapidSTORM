#ifndef DSTORM_DISPLAY_APP_H
#define DSTORM_DISPLAY_APP_H

#include <wx/wx.h>
#include <memory>
#include <set>
#include <boost/function/function0.hpp>
#include "MainThread.h"

namespace dStorm {
namespace display {

struct Window;

/** All App methods must be called from the event
 *  dispatcher thread. */
class App : public wxApp, public dStorm::MainThread {
  private:
    boost::mutex mutex;
    boost::condition main_thread_wakeup;
    std::set<Job*> active_jobs;
    int job_count;

    std::set<Window*> windows;
    wxTimer timer;

    DECLARE_EVENT_TABLE();

  public:
    App();
    ~App();

    static boost::function0<void> idle_call;

    void run( bool gui );
    /** Command events sent to the App are interpreted as
     *  ost::Runnable objects to be run in the event queue. */
    void OnIdle( wxIdleEvent& );
    void OnTimer( wxTimerEvent& );
    void close() {}
    void add_window(Window* w) { windows.insert(w); }
    void remove_window(Window* w) { windows.erase(w); }

    void run_all_jobs();
    void register_job( Job& );
    void unregister_job( Job& );
    void register_unstopable_job();
    void unregister_unstopable_job();
    void terminate_running_jobs();
    int count_jobs();
};

}
}

DECLARE_APP(dStorm::display::App);

#endif
