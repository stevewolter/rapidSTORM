#include "App.h"
#include "wxManager.h"

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
#if 0
    std::cerr << "ost thread: " << ost::Thread::description() << "\n";
    std::cerr << "Showing nevershow\n";
    nevershow->Show();
    std::cerr << "Showed nevershow\n";
    SetTopWindow( nevershow.get() );
#endif
    return true;
}

void App::OnIdle(wxIdleEvent&) {
    if ( idle_call )
        idle_call->run();
}

void App::close() {
    nevershow.release()->Destroy();
}

}
}

IMPLEMENT_APP_NO_MAIN( dStorm::Display::App );
