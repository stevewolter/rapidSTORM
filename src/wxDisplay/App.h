#ifndef DSTORM_DISPLAY_APP_H
#define DSTORM_DISPLAY_APP_H

#include <wx/wx.h>
#include <memory>
#include <dStorm/helpers/thread.h>

namespace dStorm {
namespace Display {

/** All App methods must be called from the event
 *  dispatcher thread. */
class App : public wxApp {
  private:
    std::auto_ptr<wxFrame> nevershow;

    DECLARE_EVENT_TABLE();

  public:
    App();
    ~App();

    static ost::Runnable* idle_call;

    bool OnInit(); 
    /** Command events sent to the App are interpreted as
     *  ost::Runnable objects to be run in the event queue. */
    void OnIdle( wxIdleEvent& );
    void close();
};

}
}

DECLARE_APP(dStorm::Display::App);

#endif
