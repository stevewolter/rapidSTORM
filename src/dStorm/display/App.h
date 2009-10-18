#ifndef DSTORM_DISPLAY_APP_H
#define DSTORM_DISPLAY_APP_H

#include <wx/wx.h>
#include <memory>

namespace dStorm {
namespace Display {

/** All App methods must be called from the event
 *  dispatcher thread. */
class App : public wxApp {
  private:
    std::auto_ptr<wxFrame> nevershow;
    class RunnableQueueTimer : public wxTimer {
        void Notify();
    };
    std::auto_ptr<RunnableQueueTimer> timer;

    DECLARE_EVENT_TABLE();

  public:
    class Window;

    App();

    bool OnInit(); 
    void OnIdle(wxIdleEvent& event); 
    void close();
};

}
}

DECLARE_APP(dStorm::Display::App);

#endif
