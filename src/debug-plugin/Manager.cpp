#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "Manager.h"
#include "md5.h"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <boost/units/io.hpp>
#include <simparm/ChoiceEntry_Impl.hh>
#include <dStorm/helpers/Variance.h>
#include <dStorm/image/iterator.h>
#include <dStorm/log.h>

class Manager::ControlConfig
: public simparm::Object, public simparm::Listener, private boost::noncopyable
{
    Manager& m;
  public:
    simparm::DataChoiceEntry<int> to_close;

    ControlConfig(Manager& m) 
        : Object("DummyDisplayManagerConfig", "Dummy display manager"), m(m),
          to_close("ToClose", "Close Window")
    {
        to_close.set_auto_selection( false );
        receive_changes_from( to_close.value );
        push_back( to_close );
    }

    void operator()(const simparm::Event& e) {
        if ( &e.source == &to_close.value && to_close.isValid() ) {
            int number = to_close();
            ost::MutexLock lock(m.mutex);
            SourcesMap::iterator i = m.sources_by_number.find(number);
            if ( i != m.sources_by_number.end() ) {
                i->second->handler.notice_closed_data_window();
            }
        }
    }
};

class Manager::Handle 
: public dStorm::Display::Manager::WindowHandle {
    Sources::iterator i;
    Manager& m;

  public:
    Handle( 
        const WindowProperties& properties,
        dStorm::Display::DataSource& source,
        Manager& m ) : m(m) 
    {
        guard lock(m.mutex);
        i = m.sources.insert( 
            m.sources.end(),
            Source(properties, source, m.number++) );
        m.sources_by_number.insert( make_pair( i->number, i ) );
        std::stringstream entrynum;
        entrynum << i->number;
        m.control_config->to_close.addChoice( i->number, "Window" + entrynum.str(), "Window " + entrynum.str());
        LOG( "Created new window number " << i->number );
    }
    ~Handle() {
        guard lock(m.mutex);
        m.print_status(*i, "Destructing ", true);
        m.sources.erase( i );
        m.control_config->to_close.removeChoice(i->number);
    }

    std::auto_ptr<dStorm::Display::Change>
        get_state();
};

Manager::Source::Source(
    const WindowProperties& properties,
    dStorm::Display::DataSource& source,
    int n)
: handler(source), number(n),
  wants_closing(false), may_close(false)
{
    LOG( "Listening to window " << properties.name );
    handle_resize( properties.initial_size );
}

void Manager::Source::handle_resize( 
    const dStorm::Display::ResizeChange& r)
{
    LOG( "Sizing display number " << number << " to " 
              << r.size.x() << " " << r.size.y() << " with "
              << r.key_size << " grey levels and pixel "
                 "size " << r.pixel_size );
    current_display = Image( r.size );
    current_display.fill(0);
    state.do_resize = true;
    state.resize_image = r;
    state.change_key.resize(
        state.resize_image.key_size );
}

bool Manager::Source::get_and_handle_change() {
    std::auto_ptr<dStorm::Display::Change> c
        = handler.get_changes();

    if ( c.get() == NULL ) {
        LOG( "Error in display handling: NULL change pointer" );
        return false;
    }

    bool has_changed = c->do_resize | c->do_clear |
                       c->do_change_image | 
                       (c->change_pixels.size() > 0) |
                       (c->change_key.size() > 0);
    if ( c->do_resize ) {
        handle_resize( c->resize_image );
    } 
    if ( c->do_clear ) {
        state.do_clear = true;
        state.clear_image = c->clear_image;
        current_display.fill(
            state.clear_image.background );
    } 
    if ( c->do_change_image ) {
        LOG( "Replacing image with " << c->image_change.new_image.frame_number().value() );
        current_display = c->image_change.new_image;
    }
    
    for( dStorm::Display::Change::PixelQueue::const_iterator
                i = c->change_pixels.begin(); 
                i != c->change_pixels.end(); i++)
    {
         current_display(i->x,i->y) = i->color;
    }

    for ( int i = 0; i < c->change_key.size(); i++ )
    {
        dStorm::Display::KeyChange kc
            = c->change_key[i];
        state.change_key[kc.index] = kc;
    }

    return has_changed;
}

void Manager::run() { dispatch_events(); }

std::auto_ptr<dStorm::Display::Manager::WindowHandle>
    Manager::register_data_source
(
    const WindowProperties& props,
    dStorm::Display::DataSource& handler
) {
    {
        guard lock(mutex);
        if ( !running ) {
            running = true;
            ost::Thread::start();
        }
    }
    return 
        std::auto_ptr<dStorm::Display::Manager::WindowHandle>( new Handle(props, handler, *this) );
}

std::ostream& operator<<(std::ostream& o, dStorm::Display::Color c)
{
    return ( o << int(c.red()) << " " << int(c.green()) << " " << int(c.blue()) );
}

void Manager::print_status(Source& s, std::string prefix, bool p)
{
    bool has_changed = s.get_and_handle_change();
    if (!has_changed && !p) return;
    
    md5_state_t pms;
    md5_byte_t digest[16];
    md5_init( &pms );
    md5_append( &pms, 
        (md5_byte_t*)s.current_display.ptr(),
        s.current_display.size_in_pixels()*
            sizeof(dStorm::Pixel) / sizeof(md5_byte_t));
    md5_finish( &pms, digest );

    std::stringstream sdigest;
    for (int j = 0; j < 16; j++)
        sdigest << std::hex << int(digest[j]);

    float sum = 0;
    int count = 0;
    for ( Source::Image::const_iterator j = s.current_display.begin(); 
                j != s.current_display.end(); ++j )
    {
        sum += int(j->red()) + j->blue() + j->green();
        if ( j->red() != 0 || j->blue() != 0 || j->green() != 0 )
            count++;
    }
    LOG( prefix << "window " << s.number << " with digest " << sdigest.str() << ", mean is " 
                << sum / s.current_display.size_in_pixels() << " and count of nonzero pixels is " << count
                );
}

void Manager::dispatch_events() {
    std::cerr << "Start of event dispatch routine\n";
    while ( running ) {
      {
        guard lock(mutex);
        for ( Sources::iterator i = sources.begin(); i != sources.end(); i++ )
        {
            if ( i->wants_closing ) { i->may_close = true; }
            print_status(*i, "Changing ");
        }
        gui_run.broadcast(); 
      }
#ifdef HAVE_USLEEP
      usleep(100000);
#elif HAVE_WINDOWS_H
      Sleep(100);
#endif
    }
    std::cerr << "End of event dispatch routine\n";
}

void Manager::run_in_GUI_thread( ost::Runnable* )
{
}

Manager::Manager(dStorm::Display::Manager *p)
: ost::Thread("DisplayManager"),
  gui_run(mutex),
  running(false),
  number(0),
  previous(p)
{
    control_config.reset( new ControlConfig(*this) );
}

Manager::~Manager()
{
    if ( running ) {
        running = false;
        this->join();
    }
}

void Manager::store_image(
        std::string filename,
        const dStorm::Display::Change& image )
{
    std::cerr << "Storing result image under " << filename << std::endl;
    previous->store_image(filename, image);
}

std::auto_ptr<dStorm::Display::Change>
     Manager::Handle::get_state()
{
    guard lock(m.mutex);
    m.gui_run.wait();
    std::auto_ptr<dStorm::Display::Change>
        rv( new dStorm::Display::Change(i->state) );

    if ( ! rv->do_resize )
        throw std::runtime_error("State not yet initialized");
    rv->do_change_image = true;
    rv->image_change.new_image = i->current_display;

    return rv;
}

simparm::Node* Manager::getConfig()
{
    return control_config.get();
}

