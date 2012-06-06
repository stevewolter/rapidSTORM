#define VERBOSE
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
  recursive( false ),
  idle_call( new IdleCall(*this) )
{
    App::idle_call = *idle_call;
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

std::auto_ptr<display::WindowHandle>
wxManager::register_data_source(
    const WindowProperties& properties,
    DataSource& handler
)
{
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
    {
        boost::lock_guard<boost::recursive_mutex> lock(mutex);
        run_queue.push_back( code );
    }
    wxWakeUpIdle();
    closed_all_handles.notify_all();
    DEBUG("Ran code in GUI thread");
}

void wxManager::exec_waiting_runnables() {
    DEBUG("Acquiring runnables lock");
    boost::lock_guard<boost::recursive_mutex> lock(mutex);
    if ( recursive ) return;
    recursive = true;
    while ( ! run_queue.empty() ) {
        std::auto_ptr< boost::function0<void> > f ( run_queue.pop_front().release() );
        DEBUG("Running runnable");
        try {
            (*f)();
        } catch (...) {
            recursive = false;
            throw;
        }
    }
    recursive = false;
    DEBUG("Finished executing waiting runnables");
}

wxManager& wxManager::get_singleton_instance() {
    static wxManager* singleton = new wxManager();
    return *singleton;
}

}
}
