#include "Manager.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "debug.h"

#include <boost/foreach.hpp>
#include <simparm/Message.h>
#include "Manager.h"
#include "Window.h"

namespace Eigen {

template <> class NumTraits<dStorm::Pixel>
    : public NumTraits<int> {};

}

namespace dStorm {
namespace text_stream_ui {

class Manager::Handle 
: public dStorm::display::Manager::WindowHandle {
    boost::shared_ptr<Window> my_source;
    Manager& m;

  public:
    Handle( 
        const WindowProperties& properties,
        dStorm::display::DataSource& source,
        Manager& m ) : m(m) 
    {
        my_source.reset( new Window(m, properties, source, m.number++) );
        my_source->attach_ui( m.current_ui );
        boost::lock_guard< boost::mutex > lock( m.source_list_mutex );
        m.sources_queue.push_back( my_source );
    }
    ~Handle() {
        my_source->drop_handler();
        m.request_action( boost::bind( &Window::handle_disassociation, my_source ) );
    }

    void store_current_display( dStorm::display::SaveRequest );
};

void Manager::run() { 
    dispatch_events(); 
}

std::auto_ptr<dStorm::display::Manager::WindowHandle>
    Manager::register_data_source_impl
(
    const WindowProperties& props,
    dStorm::display::DataSource& handler
) {
    if ( !running ) {
        running = true;
        ui_thread = boost::thread( &Manager::run, this );
    }
    return 
        std::auto_ptr<dStorm::display::Manager::WindowHandle>( new Handle(props, handler, *this) );
}

std::ostream& operator<<(std::ostream& o, dStorm::display::Color c)
{
    return ( o << int(c.red()) << " " << int(c.green()) << " " << int(c.blue()) );
}

void Manager::dispatch_events() {
    while ( running ) {
        boost::shared_ptr<Window> s = next_source();
        if ( s ) s->print_status();
        heed_requests();
        gui_run.notify_all(); 
#ifdef HAVE_USLEEP
        usleep(100000);
#elif HAVE_WINDOWS_H
        Sleep(100);
#endif
        DEBUG("Getting mutex for event loop");
        DEBUG("Got mutex for event loop");
    }
    DEBUG("Event loop ended, heeding last save requests");
    heed_requests();
    DEBUG("Finished event loop");
}

Manager::Manager(dStorm::display::Manager *p)
: running(false),
  number(0),
  master_object("DummyDisplayManagerConfig", "Dummy display manager"),
  previous(p)
{
}

Manager::~Manager()
{
    if ( running ) {
        running = false;
        ui_thread.join();
    }
}

void Manager::store_image_impl( const dStorm::display::StorableImage& i )
{
    assert( i.image.do_clear );
    previous->store_image(i);
    simparm::Message m("Stored image", "Storing result image under " + i.filename, simparm::Message::Info);
    m.send( current_ui );
}

void Manager::Handle::store_current_display( dStorm::display::SaveRequest s )
{
    DEBUG("Got store request to " << s.filename);
    m.request_action( boost::bind( &Window::save_window, my_source, s ) );
    DEBUG("Saved store request");
}

void Manager::attach_ui( simparm::NodeHandle at )
{
    current_ui = master_object.attach_ui( at );
}

void Manager::heed_requests() {
    Requests my_requests;
    {
        boost::lock_guard<boost::mutex> lock(request_mutex);
        std::swap( requests, my_requests );
    }
    BOOST_FOREACH( Requests::value_type& i, my_requests )
        i();
}

void Manager::request_action( boost::function0<void> request ) {
    if ( boost::this_thread::get_id() == ui_thread.get_id() ) {
        request();
    } else {
        boost::lock_guard<boost::mutex> lock(request_mutex);
        requests.push_back( request );
    }
}

std::auto_ptr< dStorm::display::Manager > make_test_plugin_manager( dStorm::display::Manager* old )
{
    return std::auto_ptr< dStorm::display::Manager >( new Manager(old) );
}

boost::shared_ptr<Window> Manager::next_source() {
    boost::lock_guard< boost::mutex > lock( source_list_mutex );
    if ( sources_queue.empty() ) return boost::shared_ptr<Window>();
    boost::shared_ptr<Window> s = sources_queue.front();
    sources_queue.erase( sources_queue.begin() );
    sources_queue.push_back( s );
    return s;
}

void Manager::remove_window_from_event_queue( boost::shared_ptr<Window> w ) {
    boost::lock_guard< boost::mutex > lock( source_list_mutex );
    sources_queue.erase( std::remove( sources_queue.begin(), sources_queue.end(), w ), sources_queue.end() );
}

}

namespace test {

dStorm::display::Manager*
make_display (dStorm::display::Manager *old)
{
    if ( getenv("DEBUGPLUGIN_LEAVE_DISPLAY") || !getenv("RAPIDSTORM_TESTPLUGIN_ENABLE") ) {
        return old;
    } else {
        std::cerr << "Test plugin loaded" << std::endl;
        return new text_stream_ui::Manager( old );
    }
}

}
}
