#include "App.h"
#include "wxManager.h"

#include "debug.h"

namespace dStorm {
namespace Display {

BEGIN_EVENT_TABLE(App, wxApp)
    EVT_IDLE(App::OnIdle)
END_EVENT_TABLE()

ost::Runnable *App::idle_call = NULL;

App::App()
{
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
        idle_call->run();
    DEBUG("Idled");
}

void App::close() {
    DEBUG("Closing");
    nevershow.release()->Destroy();
    DEBUG("Closed");
}

}
}

IMPLEMENT_APP_NO_MAIN( dStorm::Display::App );
