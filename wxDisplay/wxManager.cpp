#include "wxManager.h"
#include <wx/wx.h>
#include "App.h"
#include "Window.h"
#include <dStorm/helpers/Runnables.h>
#include <boost/thread.hpp>

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

    std::auto_ptr<Change> get_state(); 
};

struct wxManager::IdleCall
: public ost::Runnable 
{
    wxManager& m;
    IdleCall(wxManager& m) : m(m) {}
    void run() { m.exec_waiting_runnables(); }
};

wxManager::wxManager() 
: ost::Thread("wxWidgets"),
  open_handles(0),
  closed_all_handles(mutex),
  was_started( false ),
  may_close( false ),
  toolkit_available( true ),
  idle_call( new IdleCall(*this) )
{
}

struct wxManager::Closer : public ost::Runnable {
    void run() throw() { wxGetApp().close(); }
};

wxManager::~wxManager() {
    DEBUG("Stopping display thread");
    may_close = true;
    if ( toolkit_available && was_started ) {
        Closer closer;
        run_in_GUI_thread( &closer );
        closed_all_handles.signal();
        join();
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
    App::idle_call = idle_call.get();
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

struct wxManager::Creator : public ost::Runnable {
    wxManager& m;

    Creator(wxManager& m) : m(m) {}

    WindowProperties properties;
    DataSource* handler;
    WindowHandle *handle;

    void run() throw();
};

void wxManager::Creator::run() throw() {
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
            start();
        }
    }
    std::auto_ptr<Creator> creator( new Creator(*this) );
    creator->properties = properties;
    creator->handler = &handler;

    std::auto_ptr<WindowHandle> 
        handle(new WindowHandle(*this));
    increase_handle_count();
    creator->handle = handle.get();

    run_in_GUI_thread( creator.release() );

    return std::auto_ptr<Manager::WindowHandle>(
        handle.release() );
}

class wxManager::Disassociator
: public ost::WaitableRunnable
{
    WindowHandle& h;
    wxManager& m;
  public:
    Disassociator(wxManager& m, WindowHandle& handle) : h(handle), m(m) {}
    void run() throw() {
        DEBUG("Running disassociator");
        if ( h.associated_window != NULL )
            h.associated_window->remove_data_source();
        m.decrease_handle_count();

        WaitableRunnable::run();
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
    Disassociator d(m, *this);
    DEBUG("Running disassociator in GUI thread");
    m.run_in_GUI_thread( &d );
    DEBUG("Waiting for disassociator to finish");
    d.wait();
    DEBUG("Finished running disassociator");
}

class StateFetcher
: public ost::WaitableRunnable
{
    Window*& window;
    std::auto_ptr<Change> rv;
  public:
    StateFetcher(Window*& window)
        : window(window) {}

    void run() throw() {
        try {
            if ( window != NULL )
                rv = window->getState();
        } catch (const std::exception& e) {
            std::cerr << "Unable to get image from window: " << e.what() << std::endl;
        } 
        ost::WaitableRunnable::run();
    }

    std::auto_ptr<Change> result()
        { this->wait(); return rv; }
};

std::auto_ptr<Change>
wxManager::WindowHandle::get_state()
{
    StateFetcher fetcher( associated_window );
    DEBUG("Fetching results");
    m.run_in_GUI_thread( &fetcher );
    DEBUG("Fetched results");
    return fetcher.result();
}

void wxManager::run_in_GUI_thread( ost::Runnable* code ) 
{
    DEBUG("Running code in GUI thread");
    if ( ost::Thread::current_thread() == 
         static_cast<ost::Thread*>(this) )
        exec( code );
    else {
        {
            ost::MutexLock lock(mutex);
            run_queue.push( code );
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
        run_queue.pop();
    }
    DEBUG("Finished executing waiting runnables");
}

void wxManager::exec(ost::Runnable* runnable) {
    runnable->run();
}

void wxManager::disassociate_window
    ( Window *, WindowHandle* handle )
{
    handle->associated_window = NULL;
}

}
}
