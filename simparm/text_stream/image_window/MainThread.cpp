#include "simparm/text_stream/image_window/MainThread.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <boost/foreach.hpp>
#include <simparm/Message.h>
#include "simparm/text_stream/image_window/Window.h"
#include <dStorm/display/SharedDataSource.h>

namespace Eigen {

template <> class NumTraits<dStorm::Pixel>
    : public NumTraits<int> {};

}

namespace simparm {
namespace text_stream {
namespace image_window {

class MainThread::Handle 
: public dStorm::display::WindowHandle {
    MainThread& m;
    boost::shared_ptr<dStorm::display::SharedDataSource> data_source;
    boost::shared_ptr<Window> window;

  public:
    Handle( 
        const dStorm::display::WindowProperties& properties,
        dStorm::display::DataSource& source,
        MainThread& m ) 
    : m(m), data_source( new dStorm::display::SharedDataSource(source, simparm::wx_ui::ProtocolNode(NULL) ) ),
      window( new Window(m, properties, data_source, m.number++) )
    {
        window->attach_ui( m.current_ui );
        boost::lock_guard< boost::mutex > lock( m.source_list_mutex );
        m.sources_queue.push_back( window );
    }
    ~Handle() {
        data_source->disconnect();
    }

    void store_current_display( dStorm::display::SaveRequest );
};

void MainThread::run() { 
    dispatch_events(); 
}

std::auto_ptr<dStorm::display::WindowHandle>
    MainThread::register_data_source
(
    const dStorm::display::WindowProperties& props,
    dStorm::display::DataSource& handler
) {
    if ( !running ) {
        running = true;
        ui_thread = boost::thread( &MainThread::run, this );
    }
    return 
        std::auto_ptr<dStorm::display::WindowHandle>( new Handle(props, handler, *this) );
}

std::ostream& operator<<(std::ostream& o, dStorm::display::Color c)
{
    return ( o << int(c.red()) << " " << int(c.green()) << " " << int(c.blue()) );
}

void MainThread::dispatch_events() {
    while ( running ) {
        boost::shared_ptr<Window> s = next_source();
        if ( s ) s->update_window();
        heed_requests();
        gui_run.notify_all(); 
#ifdef HAVE_USLEEP
        usleep(100000);
#elif HAVE_WINDOWS_H
        Sleep(100);
#endif
    }
    heed_requests();
}

MainThread::MainThread()
: running(false),
  number(0),
  master_object("DummyDisplayManagerConfig", "Dummy display manager")
{
    master_object.set_user_level( simparm::Debug );
}

MainThread::~MainThread()
{
    if ( running ) {
        running = false;
        ui_thread.join();
    }
}

void MainThread::Handle::store_current_display( dStorm::display::SaveRequest s )
{
    m.request_action( boost::bind( &Window::save_window, window, s ) );
}

void MainThread::attach_ui( simparm::NodeHandle at )
{
    current_ui = master_object.attach_ui( at );
}

void MainThread::print( std::string message ) const {
    simparm::Message m("Image window message", message, simparm::Message::Info);
    m.send( current_ui );
}

void MainThread::heed_requests() {
    Requests my_requests;
    {
        boost::lock_guard<boost::mutex> lock(request_mutex);
        std::swap( requests, my_requests );
    }
    BOOST_FOREACH( Requests::value_type& i, my_requests )
        i();
}

void MainThread::request_action( boost::function0<void> request ) {
    if ( boost::this_thread::get_id() == ui_thread.get_id() ) {
        request();
    } else {
        boost::lock_guard<boost::mutex> lock(request_mutex);
        requests.push_back( request );
    }
}

boost::shared_ptr<Window> MainThread::next_source() {
    boost::lock_guard< boost::mutex > lock( source_list_mutex );
    if ( sources_queue.empty() ) return boost::shared_ptr<Window>();
    boost::shared_ptr<Window> s = sources_queue.front();
    sources_queue.erase( sources_queue.begin() );
    sources_queue.push_back( s );
    return s;
}

void MainThread::remove_window_from_event_queue( boost::shared_ptr<Window> w ) {
    boost::lock_guard< boost::mutex > lock( source_list_mutex );
    sources_queue.erase( std::remove( sources_queue.begin(), sources_queue.end(), w ), sources_queue.end() );
}

}
}
}
