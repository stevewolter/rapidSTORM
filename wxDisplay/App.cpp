#include <stdlib.h>
#include "App.h"
#include "wxManager.h"
#include "Window.h"

#include "debug.h"
#include "wx_ui/Launcher.h"
#include "ModuleLoader.h"

namespace dStorm {
namespace display {

DECLARE_EVENT_TYPE(DISPLAY_TIMER, -1)
DEFINE_EVENT_TYPE(DISPLAY_TIMER)

BEGIN_EVENT_TABLE(App, wxAppConsole)
    EVT_TIMER(DISPLAY_TIMER, App::OnTimer)
    EVT_IDLE(App::OnIdle)
END_EVENT_TABLE()

boost::function0<void> App::idle_call;

App::App()
: job_count(0),
  timer(this, DISPLAY_TIMER)
{
    int display_timer = 100;
    if ( getenv("RAPIDSTORM_DISPLAY_FREQUENCY") ) {
        float f = atof(getenv("RAPIDSTORM_DISPLAY_FREQUENCY"));
        if ( f > 0 )
            display_timer = 1000 / f;
    }
    DEBUG("App has been constructed");
    timer.Start( display_timer );
}

App::~App() {
}

void App::run_all_jobs() 
{
    boost::mutex::scoped_lock lock( mutex );
    while ( job_count > 0 ) {
        ProcessPendingEvents();
        if ( idle_call ) idle_call();
        //main_thread_wakeup.wait(lock);
    }
}

void App::register_job( Job& job ) {
    boost::mutex::scoped_lock lock( mutex );
    ++job_count;
    active_jobs.insert( &job );
}

void App::unregister_job( Job& job ) {
    boost::lock_guard< boost::mutex > lock( mutex );
    active_jobs.erase( &job );
    --job_count;
    if ( job_count == 0 )
        main_thread_wakeup.notify_all();
}

void App::terminate_running_jobs() {
    boost::lock_guard< boost::mutex > lock( mutex );
    DEBUG("Terminate remaining cars");
    std::for_each( active_jobs.begin(), active_jobs.end(),
                   std::mem_fun(&Job::stop) );
}

int App::count_jobs() {
    boost::lock_guard< boost::mutex > lock( mutex );
    return job_count;
}

void App::register_unstopable_job() {
    boost::lock_guard< boost::mutex > lock( mutex );
    ++job_count;
}

void App::unregister_unstopable_job() {
    boost::lock_guard< boost::mutex > lock( mutex );
    --job_count;
    if ( job_count == 0 ) main_thread_wakeup.notify_all();
}


#if 0
bool App::OnInit()
{
    if ( !wxApp::OnInit() )
        return false;

    std::cerr << "Got on init" << std::endl;
    nevershow.reset( new wxFrame(NULL, wxID_ANY, _T("Nevershow")) );
    return true;
}
#endif

void App::run( bool gui ) {
    dStorm::job::Config config;
    add_modules( config );
    simparm::wx_ui::Launcher launcher( config, *this );
    launcher.launch();
    run_all_jobs();

    return 0;
}

void App::OnIdle(wxIdleEvent&) {
    DEBUG("Idling");
    if ( idle_call )
        idle_call();
    DEBUG("Idled");
}

void App::OnTimer(wxTimerEvent& event) {
    for (std::set<Window*>::iterator i = windows.begin(); i != windows.end(); ++i)
      (*i)->update_image();
    event.Skip();
}

}
}

IMPLEMENT_APP_NO_MAIN( dStorm::display::App );
