#include <wx/wx.h>
#include "wxManager.h"
#include "App.h"
#include "Window.h"
#include <dStorm/helpers/Runnables.h>
#include <boost/thread.hpp>
#include <boost/ref.hpp>

#include "debug.h"

namespace dStorm {
namespace Display {

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
  closed_all_handles(mutex),
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
        closed_all_handles.signal();
        gui_thread.join();
    } else {
        ost::MutexLock lock(mutex);
        closed_all_handles.signal();
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

    ost::MutexLock lock( mutex );
    exec_waiting_runnables();
    while ( !may_close || open_handles > 0 ) {
        closed_all_handles.wait();
        exec_waiting_runnables();
    }
}

void wxManager::increase_handle_count() {
    ost::MutexLock lock(mutex);
    open_handles++;
}

void wxManager::decrease_handle_count() {
    ost::MutexLock lock(mutex);
    open_handles--;
    if ( open_handles == 0 && may_close ) {
        closed_all_handles.signal();
        if ( toolkit_available )
            wxGetApp().close();
    }
}

struct wxManager::Creator {
    wxManager& m;

    Creator(wxManager& m) : m(m) {}

    WindowProperties properties;
    DataSource* handler;
    WindowHandle *handle;

    void operator()();
};

void wxManager::Creator::operator()() {
    if ( m.toolkit_available ) {
        Window * w = new Window( properties, handler, handle );
        handle->associated_window = w;
    }
}

std::auto_ptr<Manager::WindowHandle>
wxManager::register_data_source(
    const WindowProperties& properties,
    DataSource& handler
)
{
    if ( ! was_started ) {
        ost::MutexLock lock( mutex );
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

    run_in_GUI_thread( creator );

    return std::auto_ptr<Manager::WindowHandle>(
        handle.release() );
}

class wxManager::Disassociator
{
    WindowHandle& h;
    wxManager& m;
  public:
    Disassociator(wxManager& m, WindowHandle& handle) : h(handle), m(m) {}
    void operator()() {
        DEBUG("Running disassociator on " << this);
        if ( h.associated_window != NULL )
            h.associated_window->remove_data_source();
        m.decrease_handle_count();
        DEBUG("Ran disassociator");
    }
};

wxManager::WindowHandle::WindowHandle(wxManager& m)
: m(m), associated_window(NULL)
{
}

wxManager::WindowHandle::~WindowHandle()
{
    DEBUG("Destructing window handle");
    /* This code must be run even if associated_window is
     * NULL to avoid race condition where associated_window
     * is not yet set. */
    Waitable<Disassociator> d (m, *this);
    DEBUG("Running disassociator in GUI thread ");
    m.run_in_GUI_thread( d );
    DEBUG("Waiting for disassociator " << &d.functor() << " to finish");
    d.wait();
    DEBUG("Finished running disassociator");
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
            wxManager().getSingleton().store_image( request.filename, *c );
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
            ost::MutexLock lock(mutex);
            run_queue.push_back( code );
        }
        wxWakeUpIdle();
        closed_all_handles.signal();
    }
    DEBUG("Ran code in GUI thread");
}

void wxManager::exec_waiting_runnables() {
    DEBUG("Acquiring runnables lock");
    ost::MutexLock lock(mutex);
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

void wxManager::disassociate_window
    ( Window *, WindowHandle* handle )
{
    handle->associated_window = NULL;
}

}
}
