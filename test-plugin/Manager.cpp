#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <dStorm/log.h>
#include "Manager.h"
#include "md5.h"
#include <iomanip>
#include <fstream>
#include <sstream>
#include <boost/units/io.hpp>
#include <simparm/ChoiceEntry_Impl.hh>
#include <simparm/Entry.hh>
#include <simparm/TriggerEntry.hh>
#include <dStorm/image/iterator.h>
#include <dStorm/image/contains.h>
#include <boost/foreach.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/bind/bind.hpp>

class Manager::ControlConfig
: public simparm::Object, public simparm::Listener, private boost::noncopyable
{
    Manager& m;
    simparm::DataChoiceEntry<int> which_window;
    simparm::Entry<unsigned long> which_key;
    simparm::Entry<unsigned long> top, bottom, left, right;
    simparm::StringEntry new_limit;
    simparm::TriggerEntry close, set_lower_limit, set_upper_limit, draw_rectangle;

  public:
    ControlConfig(Manager& m) 
        : Object("DummyDisplayManagerConfig", "Dummy display manager"), m(m),
          which_window("WhichWindow", "Select window"),
          which_key("WhichKey", "Select key", 1),
          top("RectangleTop", "Top border of drawn rectangle", 0),
          bottom("RectangleBottom", "Bottom border of drawn rectangle", 511),
          left("RectangleLeft", "Left border of drawn rectangle", 0),
          right("RectangleRight", "Right border of drawn rectangle", 511),
          new_limit("NewLimit", "New limit for selected key"),
          close("Close", "Close Window"),
          set_lower_limit("SetLowerLimit", "Set lower key limit"),
          set_upper_limit("SetUpperLimit", "Set upper key limit"),
          draw_rectangle("DrawRectangle", "Draw rectangle")
    {
        which_window.set_auto_selection( false );
        push_back( which_window );
        push_back( close );
        push_back( which_key );
        push_back( new_limit );
        push_back( set_lower_limit );
        push_back( set_upper_limit );
        push_back(top);
        push_back(bottom);
        push_back(left);
        push_back(right);
        push_back(draw_rectangle);
        receive_changes_from( close.value );
        receive_changes_from( set_lower_limit.value );
        receive_changes_from( set_upper_limit.value );
        receive_changes_from( draw_rectangle.value );
    }

    void added_choice( int n ) {
        std::stringstream entrynum;
        entrynum << n;
        which_window.addChoice( n, "Window" + entrynum.str(), "Window " + entrynum.str());
    }

    void removed_choice( int n ) {
        which_window.removeChoice(n);
    }

    void operator()(const simparm::Event& e) {
        if ( e.cause != simparm::Event::ValueChanged ) return;
        if ( ! which_window.isValid() ) {
            std::cerr << "No valid window selected" << std::endl;
            return;
        }
        boost::shared_ptr<Source> src;
        {
            boost::lock_guard<boost::recursive_mutex> lock(m.mutex);
            src = m.sources[which_window()];
        }
        if ( ! src ) { 
            std::cerr << "Window " << which_window() << " not found" << std::endl; return; 
        }

        if ( &e.source == &close.value && close.triggered() ) {
            close.untrigger();
            m.request_action( src, Close() );
        } else if ( &e.source == &set_lower_limit.value && set_lower_limit.triggered() ) {
            set_lower_limit.untrigger();
            m.request_action( src, SetLimit(true, which_key(), new_limit()) );
        } else if ( &e.source == &set_upper_limit.value && set_upper_limit.triggered() ) {
            set_upper_limit.untrigger();
            m.request_action( src, SetLimit(false, which_key(), new_limit()) );
        } else if ( &e.source == &draw_rectangle.value && draw_rectangle.triggered() ) {
            draw_rectangle.untrigger();
            m.request_action( src, DrawRectangle(left(), right(), top(), bottom()) );
        }
    }

    void processCommand( std::istream& in ); 
};

void Manager::ControlConfig::processCommand( std::istream& in )
{
        std::string command;
        in >> command;
        if ( command == "pixel_value" ) {
            DEBUG("Reading instructions");
            std::string window;
            dStorm::display::Image::Position pos;
            int number;
            Sources::iterator i = m.sources.end();
            std::stringstream msg;

            if ( which_window.isValid() ) {
                DEBUG("Finding window");
                number = which_window();
                i = m.sources.find(number);
                pos.fill(0);
                in >> boost::units::quantity_cast<int&>(pos.x()) 
                   >> boost::units::quantity_cast<int&>(pos.y());
                msg << "window " << number;
            } else {
                msg << "result for unset WhichWindow field";
            }
            if ( i != m.sources.end() ) {
                DEBUG("Found window");
                msg << " pixel at (" << pos.transpose() << ")";
                Source& source = *i->second;
                if ( source.current_display.contains( pos ) )
                    msg << " has value r " << int(source.current_display( pos ).red() )
                                   << " g " << int(source.current_display( pos ).green())
                                   << " b " << int(source.current_display( pos ).blue());
                else
                    msg << " is out of image dimensions";
            } else {
                msg << " is undefined";
            }
            DEBUG("Printing results");
            print( msg.str() );
            DEBUG("Printed results");
        } else if ( command == "in" ) {
            std::string name;
            in >> name;
            (*this)[name].processCommand(in);
        }
    }

class Manager::Handle 
: public dStorm::display::Manager::WindowHandle {
    boost::shared_ptr<Source> my_source;
    Manager& m;

  public:
    Handle( 
        const WindowProperties& properties,
        dStorm::display::DataSource& source,
        Manager& m ) : m(m) 
    {
        my_source.reset( new Source(properties, source, m.number++) );
        m.sources.insert( std::make_pair( my_source->number, my_source ) );
        m.control_config->added_choice( my_source->number );
        LOG( "Created new window number " << my_source->number << " named " << properties.name );
    }
    ~Handle() {
        Disassociation destruction;
        m.request_action( my_source, boost::ref(destruction) );
        boost::unique_lock<boost::mutex> l( destruction.m );
        while ( ! destruction.commenced )
            destruction.c.wait( l );
    }

    void store_current_display( dStorm::display::SaveRequest );
};

Manager::Source::Source(
    const WindowProperties& properties,
    dStorm::display::DataSource& source,
    int n)
: handler(source), state(0), number(n),
  wants_closing(false), may_close(false)
{
    LOG( "Listening to window " << properties.name );
    handle_resize( properties.initial_size );
}

void Manager::Source::handle_resize( 
    const dStorm::display::ResizeChange& r)
{
    LOG( "Sizing display number " << number << " to " 
              << r.size.x() << " " << r.size.y() << " with "
              << ((r.keys.empty()) ? 0 : r.keys.front().size) << " grey levels and pixel "
              << "size " << r.pixel_sizes[0].value << " " << r.pixel_sizes[0].unit_symbol << " in x "
              << "and " << r.pixel_sizes[1].value << " " << r.pixel_sizes[1].unit_symbol << " in y " );
    for (unsigned int i = 0; i < r.keys.size(); ++i ) {
        LOG( "Window " << number << " key " << i << " is measured in " << r.keys[i].unit << " and has " << r.keys[i].size << " levels and the user "
             "can " << ((r.keys[i].can_set_lower_limit) ? "" : "not ") << "set the lower limit" );
    }
    current_display = Image( r.size );
    current_display.fill(0);
    state.do_resize = true;
    state.resize_image = r;
    state.changed_keys.resize( r.keys.size() );
    for (size_t i = 0; i < r.keys.size(); ++i)
        state.changed_keys[i].resize( r.keys[i].size );
}

bool Manager::Source::get_and_handle_change() {
    std::auto_ptr<dStorm::display::Change> c
        = handler.get_changes();

    if ( c.get() == NULL ) {
        LOG( "Error in display handling: NULL change pointer" );
        return false;
    }

    bool has_changed = c->do_resize || c->do_clear ||
                       c->do_change_image || 
                       (c->change_pixels.size() > 0);
    for (dStorm::display::Change::Keys::const_iterator i = c->changed_keys.begin(); 
                                      i != c->changed_keys.end(); ++i)
        has_changed = has_changed || (i->size() > 0);

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
    
    for( dStorm::display::Change::PixelQueue::const_iterator
                i = c->change_pixels.begin(); 
                i != c->change_pixels.end(); i++)
    {
         current_display(*i) = i->color;
    }

    for ( size_t j = 0; j < c->changed_keys.size(); ++j)
        for ( size_t i = 0; i < c->changed_keys[j].size(); i++ )
        {
            dStorm::display::KeyChange kc = c->changed_keys[j][i];
            if ( i == 31 )
                LOG("Window " << number << " key " << j << " value 31 has value " << kc.value);
            state.changed_keys[j][kc.index] = kc;
        }

    return has_changed;
}

void Manager::run() { 
    DEBUG("Running window manager subthread");
    dispatch_events(); 
}

std::auto_ptr<dStorm::display::Manager::WindowHandle>
    Manager::register_data_source_impl
(
    const WindowProperties& props,
    dStorm::display::DataSource& handler
) {
    boost::lock_guard< boost::recursive_mutex > lock(mutex);
    if ( !running ) {
        DEBUG("Forking window manager subthread");
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
    DEBUG("Event loop");
    boost::unique_lock< boost::recursive_mutex > lock(mutex);
    DEBUG("Got mutex for event loop");
    while ( running ) {
        DEBUG("Running GUI loop");
        for ( Sources::iterator i = sources.begin(); i != sources.end(); i++ )
        {
            if ( i->second->wants_closing ) { i->second->may_close = true; }
            print_status(*i->second, "Changing ");
        }
        heed_requests();
        DEBUG("Ran GUI loop");
        gui_run.notify_all(); 
        lock.unlock();
#ifdef HAVE_USLEEP
        usleep(100000);
#elif HAVE_WINDOWS_H
        Sleep(100);
#endif
        DEBUG("Getting mutex for event loop");
        lock.lock();
        DEBUG("Got mutex for event loop");
    }
    DEBUG("Event loop ended, heeding last save requests");
    heed_requests();
    DEBUG("Finished event loop");
}

Manager::Manager(dStorm::display::Manager *p)
: running(false),
  number(0),
  previous(p)
{
    control_config.reset( new ControlConfig(*this) );
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
    std::cerr << "Storing result image under " << i.filename << std::endl;
    previous->store_image(i);
}

void Manager::Handle::store_current_display( dStorm::display::SaveRequest s )
{
    DEBUG("Got store request to " << s.filename);
    m.request_action( my_source, s );
    DEBUG("Saved store request");
}

simparm::Node* Manager::getConfig()
{
    return control_config.get();
}

void Manager::heed_requests() {
    Requests my_requests;
    {
        boost::lock_guard<boost::mutex> lock(request_mutex);
        std::swap( requests, my_requests );
    }
    BOOST_FOREACH( Requests::value_type& i, my_requests )
        if ( sources.find( i.first->number ) != sources.end() )
            boost::apply_visitor( boost::bind( boost::ref(*i.first), _1, boost::ref(*this) ), i.second );
}

void Manager::Source::operator()( const dStorm::display::SaveRequest& i, Manager& m ) {
    std::auto_ptr<dStorm::display::Change>
        rv( new dStorm::display::Change(state) );
    if ( ! rv->do_resize )
        throw std::runtime_error("State not yet initialized");
    rv->do_change_image = true;
    rv->image_change.new_image = current_display;

    if ( i.manipulator )
        i.manipulator(*rv);
    dStorm::display::StorableImage result( i.filename, *rv );
    result.scale_bar = i.scale_bar;
    m.store_image( result );
}

void Manager::Source::operator()( const Close&, Manager& ) {
    handler.notice_closed_data_window();
}

void Manager::Source::operator()( const SetLimit& i, Manager& ) {
    std::cerr << "Noticing key limits for key " << i.key << " " << i.lower_limit << " " << i.limit << std::endl;
    handler.notice_user_key_limits( i.key, i.lower_limit, i.limit );
}

void Manager::Source::operator()( const DrawRectangle& i, Manager& ) {
    handler.notice_drawn_rectangle( i.l, i.r, i.t, i.b );
}

void Manager::Source::operator()(Disassociation& d, Manager& m ) {
    m.print_status(*this, "Destructing ", true);
    m.control_config->removed_choice(number);
    m.sources.erase( number );

    boost::lock_guard<boost::mutex> l(d.m);
    d.commenced = true;
    d.c.notify_all();
}

void Manager::request_action( boost::shared_ptr<Source>& on, const Request& request ) {
    if ( boost::this_thread::get_id() == ui_thread.get_id() ) {
        if ( sources.find( on->number ) != sources.end() )
            boost::apply_visitor( boost::bind( boost::ref(*on), _1, boost::ref(*this) ), request );
    } else {
        boost::lock_guard<boost::mutex> lock(request_mutex);
        requests.push_back( std::make_pair( on, request ) );
    }
}
