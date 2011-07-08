#ifndef DSTORM_DISPLAY_APP_H
#define DSTORM_DISPLAY_APP_H

#include <wx/wx.h>
#include <memory>
#include <boost/function/function0.hpp>

namespace dStorm {
namespace Display {

struct Window;

/** All App methods must be called from the event
 *  dispatcher thread. */
class App : public wxApp {
  private:
    std::auto_ptr<wxFrame> nevershow;
    std::set<Window*> windows;
    wxTimer timer;

    DECLARE_EVENT_TABLE();

  public:
    App();
    ~App();

    static boost::function0<void> idle_call;

    bool OnInit(); 
    /** Command events sent to the App are interpreted as
     *  ost::Runnable objects to be run in the event queue. */
    void OnIdle( wxIdleEvent& );
    void close();
    void add_window(Window* w) { windows.insert(w); }
    void remove_window(Window* w) { windows.erase(w); }
};

}
}

DECLARE_APP(dStorm::Display::App);

#endif
