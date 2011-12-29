#include "App.h"
#include "wxManager.h"
#include "Window.h"

#include "debug.h"

namespace dStorm {
namespace display {

DECLARE_EVENT_TYPE(DISPLAY_TIMER, -1)
DEFINE_EVENT_TYPE(DISPLAY_TIMER)

BEGIN_EVENT_TABLE(App, wxApp)
    EVT_TIMER(DISPLAY_TIMER, App::OnTimer)
    EVT_IDLE(App::OnIdle)
END_EVENT_TABLE()

boost::function0<void> App::idle_call;

App::App()
: timer(this, DISPLAY_TIMER)
{
    DEBUG("App has been constructed");
    timer.Start( 100 );
}

App::~App() {
}

bool App::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    nevershow.reset( new wxFrame(NULL, wxID_ANY, _T("Nevershow")) );
    return true;
}

void App::OnIdle(wxIdleEvent&) {
    DEBUG("Idling");
    if ( idle_call )
        idle_call();
    DEBUG("Idled");
}

void App::close() {
    DEBUG("Closing");
    if ( nevershow.get() )
        nevershow.release()->Destroy();
    DEBUG("Closed");
}

void App::OnTimer(wxTimerEvent& event) {
    for (std::set<Window*>::iterator i = windows.begin(); i != windows.end(); ++i)
      (*i)->update_image();
    event.Skip();
}

}
}

IMPLEMENT_APP_NO_MAIN( dStorm::display::App );
