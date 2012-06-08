#ifndef DSTORM_DISPLAY_APP_H
#define DSTORM_DISPLAY_APP_H

#include <wx/wx.h>
#include <memory>
#include <set>

namespace simparm {
namespace wx_ui {

class App : public wxApp {
  private:
    DECLARE_EVENT_TABLE();

  public:
    App();
    ~App();

    /** Command events sent to the App are interpreted as
     *  ost::Runnable objects to be run in the event queue. */
    bool OnInit();
    void OnIdle( wxIdleEvent& );
};

}
}

DECLARE_APP(simparm::wx_ui::App);

#endif
