#include <wx/wx.h>
#include "wxManager.h"
#include "App.h"
#include "Window.h"
#include <boost/thread.hpp>
#include <boost/ref.hpp>

#include "debug.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <boost/utility.hpp>

namespace dStorm {

namespace display {

struct wxManager::WindowHandle
: public Manager::WindowHandle
{
    wxManager& m;
    Window *associated_window;

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

struct wxManager::Creator {
    wxManager& m;

    Creator(wxManager& m) : m(m), did_run(false) {}

    WindowProperties properties;
    DataSource* handler;
    WindowHandle *handle;

    bool did_run;
    boost::mutex did_run_mutex;
    boost::condition did_run_condition;

    void operator()();
};

void wxManager::Creator::operator()() {
    if ( m.toolkit_available ) {
        Window * w = new Window( properties, handler, handle );
        handle->associated_window = w;
    }
    boost::mutex::scoped_lock lock( did_run_mutex );
    did_run = true;
    did_run_condition.notify_all();

}

std::auto_ptr<Manager::WindowHandle>
wxManager::register_data_source_impl(
    const WindowProperties& properties,
    DataSource& handler
)
{
    if ( ! was_started ) {
        boost::lock_guard<boost::recursive_mutex> lock( mutex );
        if ( ! was_started ) {
            was_started = true;
            gui_thread = boost::thread( &wxManager::run, this );
        }
    }
    Creator creator(*this);
    creator.properties = properties;
    creator.handler = &handler;

    std::auto_ptr<WindowHandle> 
        handle(new WindowHandle(*this));
    increase_handle_count();
    creator.handle = handle.get();

    run_in_GUI_thread( boost::ref(creator) );

    boost::mutex::scoped_lock lock( creator.did_run_mutex );
    while ( ! creator.did_run )
        creator.did_run_condition.wait(lock);

    return std::auto_ptr<Manager::WindowHandle>(
        handle.release() );
}

wxManager::WindowHandle::WindowHandle(wxManager& m)
: m(m), associated_window(NULL)
{
}

wxManager::WindowHandle::~WindowHandle()
{
    if ( associated_window ) {
        boost::shared_ptr<const Change> last_changes = 
            associated_window->detach_from_source();
        m.run_in_GUI_thread( boost::bind( &Window::notice_that_source_has_disappeared, associated_window, last_changes ) );
    }
    m.decrease_handle_count();
}

class StateFetcher
{
    wxManager::WindowHandle& handle;
    SaveRequest request;
  public:
    StateFetcher(wxManager::WindowHandle& handle, const SaveRequest& r )
        : handle(handle), request(r) {}

    void operator()() {
        try {
            if ( ! handle.associated_window ) throw std::runtime_error("Window already destructed");
            Window& window = *handle.associated_window;
            std::auto_ptr<Change> c = window.getState();
            if ( request.manipulator ) request.manipulator(*c);
            wxManager::getSingleton().store_image( request.filename, *c );
        } catch (const std::runtime_error& e) {
            std::cerr << "Unable to save image: " << e.what() << std::endl;
        } 
    }
};

void wxManager::WindowHandle::store_current_display( SaveRequest s )
{
    m.run_in_GUI_thread( StateFetcher(*this, s) );
}

void wxManager::run_in_GUI_thread( std::auto_ptr<Runnable> code ) 
{
    DEBUG("Running code in GUI thread");
    if ( boost::this_thread::get_id() == gui_thread.get_id() )
        exec( *code );
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
        exec( run_queue.front() );
        DEBUG("Ran runnable");
        run_queue.pop_front();
    }
    DEBUG("Finished executing waiting runnables");
}

void wxManager::exec(Runnable& runnable) {
    runnable();
}

std::auto_ptr< Manager > make_wx_manager() {
    return std::auto_ptr< Manager >( new wxManager() );
}

}
}
