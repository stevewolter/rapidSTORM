#include "wxManager.h"
#include <wx/wx.h>
#include "App.h"
#include "Window.h"
#include <dStorm/helpers/Runnables.h>

namespace dStorm {
namespace Display {

struct wxManager::WindowHandle
: public Manager::WindowHandle
{
    Window *associated_window;
    WindowHandle() : associated_window(NULL) {}
    ~WindowHandle();
};

wxManager::wxManager() 
: ost::Thread("wxWidgets")
{
    detach();
}

void wxManager::run() throw()
{
    int argc = 0;
    wxEntry(argc, (wxChar**)NULL);
}

struct Closer : public ost::Runnable {
    void run() throw() { wxGetApp().close(); }
    void finish() { delete this; }
};

void wxManager::close() throw()
{
    run_in_GUI_thread( new Closer() );
}

struct wxManager::Creator : public ost::Runnable {
    WindowProperties properties;
    DataSource* handler;
    WindowHandle *handle;

    void run() throw();
};

void wxManager::Creator::run() throw() {
    Window * w = new Window( properties, handler, handle );
    handle->associated_window = w;
}

std::auto_ptr<Manager::WindowHandle>
wxManager::register_data_source(
    const WindowProperties& properties,
    DataSource& handler
)
{
    std::auto_ptr<Creator> creator( new Creator() );
    creator->properties = properties;
    creator->handler = &handler;

    std::auto_ptr<WindowHandle> 
        handle(new WindowHandle());
    creator->handle = handle.get();

    run_in_GUI_thread( creator.release() );

    return std::auto_ptr<Manager::WindowHandle>(
        handle.release() );
}

class wxManager::Disassociator
: public ost::WaitableRunnable
{
    WindowHandle& h;
  public:
    Disassociator(WindowHandle& handle) : h(handle) {}
    void run() throw() {
        if ( h.associated_window != NULL )
            h.associated_window->remove_data_source();
    }
};

wxManager::WindowHandle::~WindowHandle()
{
    /* This code must be run even if associated_window is
     * NULL to avoid race condition. */
    Disassociator d(*this);
    Manager::getSingleton().run_in_GUI_thread( &d );
    d.wait();
}

void wxManager::run_in_GUI_thread( ost::Runnable* code ) 
{
    if ( ost::Thread::current_thread() == 
         static_cast<ost::Thread*>(this) )
        exec( code );
    else {
        {
            ost::MutexLock lock(mutex);
            run_queue.push( code );
        }
        wxWakeUpIdle();
    }
}

void wxManager::exec_waiting_runnables() {
    ost::MutexLock lock(mutex);
    while ( ! run_queue.empty() ) {
        exec( run_queue.front() );
        run_queue.pop();
    }
}

void wxManager::exec(ost::Runnable* runnable) {
    runnable->initial();
    runnable->run();
    runnable->final();
}

void wxManager::disassociate_window
    ( Window *window, WindowHandle* handle )
{
    handle->associated_window = NULL;
}

wxManager& wxManager::getSingleton() {
    static wxManager* handler = new wxManager();
    return *handler;
}

}
}
