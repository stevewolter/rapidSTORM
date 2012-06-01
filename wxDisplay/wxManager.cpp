#include <wx/wx.h>
#include "wxManager.h"
#include "App.h"
#include "Window.h"
#include <boost/thread.hpp>
#include <boost/ref.hpp>

#include "debug.h"

#include <boost/utility.hpp>
#include <dStorm/display/store_image.h>

namespace dStorm {

namespace display {

struct wxManager::WindowHandle
: public display::WindowHandle
{
    wxManager& m;
    boost::shared_ptr< SharedDataSource > data_source;
    boost::shared_ptr< Window* > associated_window;

    WindowHandle(wxManager& m);
    ~WindowHandle();

    void store_current_display( SaveRequest );
};

struct wxManager::IdleCall
{
    wxManager& m;
    IdleCall(wxManager& m) : m(m) {}
    void operator()() {
        DEBUG("Idle call");
        m.exec_waiting_runnables(); 
    }
};

wxManager::wxManager() 
: open_handles(0),
  closed_all_handles(),
  was_started( false ),
  may_close( false ),
  toolkit_available( true ),
  idle_call( new IdleCall(*this) )
{
}

struct wxManager::Closer {
    void operator()() { 
        DEBUG("Closing wxGetApp");
        wxGetApp().close(); 
        DEBUG("Closed wxGetApp");
    }
};

wxManager::~wxManager() {
    DEBUG("Stopping display thread");
    may_close = true;
    if ( toolkit_available && was_started ) {
        DEBUG("Stopping display thread");
        run_in_GUI_thread( Closer() );
        DEBUG("Stopped display thread");
        closed_all_handles.notify_all();
        gui_thread.join();
    } else {
        boost::lock_guard<boost::recursive_mutex> lock(mutex);
        closed_all_handles.notify_all();
    }
    DEBUG("Stopped display thread");
}

void wxManager::run() throw()
{
    DEBUG("Running display thread");
    int argc = 0;
    App::idle_call = *idle_call;
    assert( App::idle_call );
    if ( !may_close )
        wxEntry(argc, (wxChar**)NULL);
    DEBUG("Ran display thread");
    toolkit_available = false;

    DEBUG("Locking job queue mutex");
    boost::unique_lock<boost::recursive_mutex> lock( mutex );
    DEBUG("Executing waiting runnables");
    exec_waiting_runnables();
    while ( !may_close || open_handles > 0 ) {
        closed_all_handles.wait(mutex);
        exec_waiting_runnables();
    }
}

void wxManager::increase_handle_count() {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    open_handles++;
}

void wxManager::decrease_handle_count() {
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    open_handles--;
    if ( open_handles == 0 && may_close ) {
        closed_all_handles.notify_all();
        if ( toolkit_available )
            wxGetApp().close();
    }
}

static void create_window( 
    const bool& toolkit_available, 
    boost::shared_ptr<Window*> window_handle_store,
    display::WindowProperties properties,
    boost::shared_ptr<SharedDataSource> data_source ) 
{
    if ( toolkit_available ) {
        *window_handle_store = new Window( properties, data_source );
    }
}

void wxManager::start_GUI_thread() {
    if ( ! was_started ) {
        DEBUG("Acquiring lock for checking if GUI thread is started");
        boost::lock_guard<boost::recursive_mutex> lock( mutex );
        DEBUG("Acquired lock for checking if GUI thread is started");
        if ( ! was_started ) {
            was_started = true;
            gui_thread = boost::thread( &wxManager::run, this );
        }
    }
}

std::auto_ptr<display::WindowHandle>
wxManager::register_data_source(
    const WindowProperties& properties,
    DataSource& handler
)
{
    start_GUI_thread();

    std::auto_ptr<WindowHandle> 
        handle(new WindowHandle(*this));
    increase_handle_count();
    handle->data_source.reset( new SharedDataSource(handler) );

    run_in_GUI_thread( 
        boost::bind(
            &create_window, 
            boost::cref( toolkit_available ),
            handle->associated_window,
            properties,
            handle->data_source ) );

    return std::auto_ptr<display::WindowHandle>(
        handle.release() );
}

wxManager::WindowHandle::WindowHandle(wxManager& m)
: m(m), associated_window( new Window*(NULL) )
{
}

wxManager::WindowHandle::~WindowHandle()
{
    data_source->disconnect();
    m.decrease_handle_count();
}

static void fetch_state( boost::shared_ptr<Window*> handle, const SaveRequest& r ) {
    try {
        Window& window = **handle;
        std::auto_ptr<Change> c = window.getState();
        if ( r.manipulator ) r.manipulator(*c);
        store_image( r.filename, *c );
    } catch (const std::runtime_error& e) {
        std::cerr << "Unable to save image: " << e.what() << std::endl;
    } 
}

void wxManager::WindowHandle::store_current_display( SaveRequest s )
{
    m.run_in_GUI_thread( boost::bind( &fetch_state, associated_window, s ) );
}

void wxManager::run_in_GUI_thread( std::auto_ptr<Runnable> code ) 
{
    DEBUG("Entering run_in_GUI_thread");
    start_GUI_thread();

    DEBUG("Running code in GUI thread");
    if ( boost::this_thread::get_id() == gui_thread.get_id() )
        (*code)();
    else {
        {
            boost::lock_guard<boost::recursive_mutex> lock(mutex);
            run_queue.push_back( code );
        }
        wxWakeUpIdle();
        closed_all_handles.notify_all();
    }
    DEBUG("Ran code in GUI thread");
}

void wxManager::exec_waiting_runnables() {
    DEBUG("Acquiring runnables lock");
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    while ( ! run_queue.empty() ) {
        DEBUG("Running runnable");
        run_queue.front()();
        DEBUG("Ran runnable");
        run_queue.pop_front();
    }
    DEBUG("Finished executing waiting runnables");
}

wxManager& wxManager::get_singleton_instance() {
    static wxManager* singleton = new wxManager();
    return *singleton;
}

}
}
