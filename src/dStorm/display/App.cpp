#include "App.h"
#include "wxManager.h"

namespace dStorm {
namespace Display {

BEGIN_EVENT_TABLE(App, wxApp)
    EVT_IDLE(App::OnIdle)
END_EVENT_TABLE()

App::App() {
}

bool App::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    nevershow.reset( new wxFrame(NULL, wxID_ANY, _T("Nevershow")) );
    timer.reset( new RunnableQueueTimer() );
    timer->Start( 50 );
    return true;
}

void App::OnIdle(wxIdleEvent& event) {
    wxManager::getSingleton().exec_GUI_thread_runnables();
    event.Skip();
}

void App::RunnableQueueTimer::Notify()
{
    wxManager::getSingleton().exec_GUI_thread_runnables();
}

void App::close() {
    nevershow.reset( NULL );
}

}
}

IMPLEMENT_APP_NO_MAIN( dStorm::Display::App );
