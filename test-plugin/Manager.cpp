#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <fstream>
#include <iomanip>
#include <map>
#include <set>

#include <boost/bind/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/optional/optional.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include <boost/units/io.hpp>
#include <boost/utility.hpp>
#include <boost/variant/apply_visitor.hpp>
#include <boost/variant/static_visitor.hpp>
#include <boost/variant/variant.hpp>
#include <boost/lexical_cast.hpp>

#include <dStorm/display/Manager.h>
#include <dStorm/image/constructors.h>
#include <dStorm/image/contains.h>
#include <dStorm/image/iterator.h>
#include <dStorm/Job.h>
#include <dStorm/log.h>
#include <dStorm/stack_realign.h>

#include <Eigen/Core>

#include <simparm/ChoiceEntry_Impl.h>
#include <simparm/Entry.h>
#include <simparm/TriggerEntry.h>
#include <simparm/text_stream/Node.h>
#include <simparm/text_stream/InnerBackendNode.h>
#include <sstream>

#include "Manager.h"
#include "md5.h"
namespace Eigen {

template <> class NumTraits<dStorm::Pixel>
    : public NumTraits<int> {};

}

class Manager 
: public dStorm::display::Manager
{
    class Handle;
    class Source;

    std::auto_ptr<WindowHandle>
        register_data_source_impl
        (const WindowProperties& properties,
         dStorm::display::DataSource& handler);

    struct Disassociation {
        Disassociation() {}
    };
    struct Close { };
    struct SetLimit { 
        bool lower_limit; int key; std::string limit;
        SetLimit(bool b, int key, std::string limit) : lower_limit(b), key(key), limit(limit) {} };
    struct DrawRectangle { int l, r, t, b; DrawRectangle(int l, int r, int t, int b) : l(l), r(r), t(t), b(b) {} };

    typedef boost::variant< Close, SetLimit, DrawRectangle, dStorm::display::SaveRequest, Disassociation > Request;

    boost::mutex source_list_mutex;
    boost::mutex request_mutex;
    boost::condition gui_run;
    bool running;
    int number;

    simparm::Object master_object;
    simparm::NodeHandle current_ui;

    std::vector< boost::shared_ptr<Source> > sources_queue;
    typedef std::vector< std::pair<boost::shared_ptr<Source>, Request> >
        Requests;
    Requests requests;
    
    std::auto_ptr<dStorm::display::Manager>
        previous;
    boost::thread ui_thread;

    boost::shared_ptr<Source> next_source();
    void dispatch_events();
    DSTORM_REALIGN_STACK void run();

    void heed_requests();

    void store_image_impl( const dStorm::display::StorableImage& );

  public:
    Manager(dStorm::display::Manager *p);
    Manager(const Manager&);
    ~Manager();

    void attach_ui( simparm::NodeHandle );

    void stop() {}
    void request_action( boost::shared_ptr<Source> on, const Request& request );
};

class Manager::Source : public boost::static_visitor<void> ,
    public boost::enable_shared_from_this<Source>
{
    Manager& m;
    boost::mutex handler_mutex;
    dStorm::display::DataSource* handler;
public:
    typedef dStorm::display::Image Image;
    Image current_display;
    dStorm::display::Change state;
    int number;

private:
    simparm::Object window_object;
    simparm::Entry<std::string> digest;
    simparm::Entry<float> mean;
    simparm::Entry<int> nonzero_count;
    simparm::Entry<int> frame_number;
    simparm::Entry<float> key_31;
    simparm::Entry<int> window_width;
    simparm::Entry<unsigned long> which_key;
    simparm::Entry<unsigned long> top, bottom, left, right;
    simparm::StringEntry new_limit;
    simparm::TriggerEntry close, set_lower_limit, set_upper_limit, draw_rectangle;
    simparm::BaseAttribute::ConnectionStore listening[4];

    class GUINode;
    std::auto_ptr< GUINode > gui_node;

public:
    Source( Manager& m,
            const WindowProperties& properties,
            dStorm::display::DataSource& source,
            int number);
    void handle_resize( 
        const dStorm::display::ResizeChange& );
    bool get_and_handle_change();

    void operator()( const Close&, Manager& m );
    void operator()( const SetLimit&, Manager& m );
    void operator()( const DrawRectangle&, Manager& m );
    void operator()( const dStorm::display::SaveRequest&, Manager& m );
    void operator()( const Disassociation&, Manager& m );

    void attach_ui( simparm::NodeHandle at );
    void print_status(std::string prefix, bool force_print = false);

    void drop_window() {
        boost::lock_guard< boost::mutex > lock( handler_mutex );
        handler = NULL;
    }

private:
    void close_window() {
        if ( close.triggered() ) {
            close.untrigger();
            m.request_action( shared_from_this(), Close() );
        }
    }
    
    void notice_lower_limit() {
        if ( set_lower_limit.triggered() ) {
            set_lower_limit.untrigger();
            m.request_action( shared_from_this(), SetLimit(true, which_key(), new_limit()) );
        }
    }

    void notice_upper_limit() {
        if ( set_upper_limit.triggered() ) {
            set_upper_limit.untrigger();
            m.request_action( shared_from_this(), SetLimit(false, which_key(), new_limit()) );
        }
    }
    
    void notice_drawn_rectangle() {
        if ( draw_rectangle.triggered() ) {
            draw_rectangle.untrigger();
            m.request_action( shared_from_this(), DrawRectangle(left(), right(), top(), bottom()) );
        }
    }

};
class Manager::Source::GUINode : public simparm::text_stream::Node {
    Source& c;
    struct Backend : public simparm::text_stream::InnerBackendNode {
        Source& c;
        void process_command_( const std::string&, std::istream& );
        Backend( GUINode& g, boost::shared_ptr<BackendNode> parent )
            : InnerBackendNode("PixelQuery", "Object", g, parent ), c(g.c) {}
    };
public:
    GUINode( Source& c, simparm::NodeHandle parent ) 
    : simparm::text_stream::Node("PixelQuery", "Object"), c(c) {
        simparm::text_stream::Node* p = dynamic_cast< simparm::text_stream::Node* >(parent.get());
        if ( p )
            set_backend_node( std::auto_ptr<simparm::text_stream::BackendNode>(
                new Backend( *this, p->get_backend() ) ) );
    }
};

void Manager::Source::GUINode::Backend::process_command_( const std::string& command, std::istream& in )
{
    if ( command == "pixel_value" ) {
        std::string window;
        dStorm::display::Image::Position pos;
        boost::shared_ptr<Source> i = c.shared_from_this();
        std::stringstream msg;

        int x, y;
        in >> x >> y;
        pos.x() = x * boost::units::camera::pixel;
        pos.y() = y * boost::units::camera::pixel;

        msg << " pixel at (" << pos.transpose() << ")";
        if ( i->current_display.contains( pos ) )
            msg << " has value r " << int(i->current_display( pos ).red() )
                            << " g " << int(i->current_display( pos ).green())
                            << " b " << int(i->current_display( pos ).blue());
        else
            msg << " is out of image dimensions";
        print( msg.str() );
    } else {
        InnerBackendNode::process_command_( command, in );
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
        my_source.reset( new Source(m, properties, source, m.number++) );
        my_source->attach_ui( m.current_ui );
        boost::lock_guard< boost::mutex > lock( m.source_list_mutex );
        m.sources_queue.push_back( my_source );
    }
    ~Handle() {
        Disassociation destruction;
        my_source->drop_window();
        m.request_action( my_source, destruction );
    }

    void store_current_display( dStorm::display::SaveRequest );
};

Manager::Source::Source(
    Manager& m,
    const WindowProperties& properties,
    dStorm::display::DataSource& source,
    int n)
: m(m), handler(&source), state(0), number(n),
  window_object( "Window" + boost::lexical_cast<std::string>(n),
                  properties.name ),
  digest("Digest", "Digest of window contents", ""),
  mean("Mean", "Mean intensity", 0),
  nonzero_count("NonzeroCount", "Count of nonzero pixels", 0),
  frame_number("FrameNumber", "Current frame number", 0),
  key_31("KeyThirtyOne", "Value for intensity 31", 0),
  window_width("WindowWidth", "Window width", 0),
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
    handle_resize( properties.initial_size );
}

void Manager::Source::attach_ui( simparm::NodeHandle at ) {
    listening[0] = close.value.notify_on_value_change( 
        boost::bind( &Source::close_window, this ) );
    listening[1] = set_lower_limit.value.notify_on_value_change( 
        boost::bind( &Source::notice_lower_limit, this ) );
    listening[2] = set_upper_limit.value.notify_on_value_change(
        boost::bind( &Source::notice_upper_limit, this ) );
    listening[3] = draw_rectangle.value.notify_on_value_change(
        boost::bind( &Source::notice_drawn_rectangle, this ) );

    simparm::NodeHandle r = window_object.attach_ui( at );
    close.attach_ui( r );
    which_key.attach_ui( r );
    new_limit.attach_ui( r );
    set_lower_limit.attach_ui( r );
    set_upper_limit.attach_ui( r );
    top.attach_ui( r);
    bottom.attach_ui( r);
    left.attach_ui( r);
    right.attach_ui( r);
    draw_rectangle.attach_ui( r);
    digest.attach_ui( r );
    mean.attach_ui( r );
    nonzero_count.attach_ui( r );
    frame_number.attach_ui( r );
    key_31.attach_ui( r );
    window_width.attach_ui( r );

    gui_node.reset( new GUINode( *this, r ) );
}


void Manager::Source::handle_resize( 
    const dStorm::display::ResizeChange& r)
{
    window_width = r.size.x().value();
#if 0
    LOG( "Sizing display number " << number << " to " 
              << r.size.x() << " " << r.size.y() << " with "
              << ((r.keys.empty()) ? 0 : r.keys.front().size) << " grey levels and pixel "
              << "size " << r.pixel_sizes[0].value << " " << r.pixel_sizes[0].unit_symbol << " in x "
              << "and " << r.pixel_sizes[1].value << " " << r.pixel_sizes[1].unit_symbol << " in y " );
    for (unsigned int i = 0; i < r.keys.size(); ++i ) {
        LOG( "Window " << number << " key " << i << " is measured in " << r.keys[i].unit << " and has " << r.keys[i].size << " levels and the user "
             "can " << ((r.keys[i].can_set_lower_limit) ? "" : "not ") << "set the lower limit" );
    }
#endif
    current_display = Image( r.size );
    current_display.fill(0);
    state.do_resize = true;
    state.resize_image = r;
    state.changed_keys.resize( r.keys.size() );
    for (size_t i = 0; i < r.keys.size(); ++i)
        state.changed_keys[i].resize( r.keys[i].size );
}

bool Manager::Source::get_and_handle_change() {
    std::auto_ptr<dStorm::display::Change> c;
    {
        boost::lock_guard<boost::mutex> lock( handler_mutex );
        if ( handler )
            c = handler->get_changes();
    }
    if ( !c.get() ) return false;

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
        frame_number = c->image_change.new_image.frame_number().value();
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
                key_31 = kc.value;
            state.changed_keys[j][kc.index] = kc;
        }

    return has_changed;
}

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

void Manager::Source::print_status(std::string prefix, bool p)
{
    bool has_changed = get_and_handle_change();
    if (!has_changed && !p) return;
    
    md5_state_t pms;
    md5_byte_t digest[16];
    md5_init( &pms );
    md5_append( &pms, 
        (md5_byte_t*)current_display.ptr(),
        current_display.size_in_pixels()*
            sizeof(dStorm::Pixel) / sizeof(md5_byte_t));
    md5_finish( &pms, digest );

    std::stringstream sdigest;
    for (int j = 0; j < 16; j++)
        sdigest << std::hex << int(digest[j]);

    float sum = 0;
    int count = 0;
    for ( Image::const_iterator j = current_display.begin(); 
                j != current_display.end(); ++j )
    {
        sum += int(j->red()) + j->blue() + j->green();
        if ( j->red() != 0 || j->blue() != 0 || j->green() != 0 )
            count++;
    }
    this->digest = sdigest.str();
    this->mean = sum / current_display.size_in_pixels();
    this->nonzero_count = count;
}

void Manager::dispatch_events() {
    while ( running ) {
        boost::shared_ptr<Source> s = next_source();
        if ( s ) s->print_status("Changing ");
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
    std::cerr << "Storing result image under " << i.filename << std::endl;
    previous->store_image(i);
}

void Manager::Handle::store_current_display( dStorm::display::SaveRequest s )
{
    DEBUG("Got store request to " << s.filename);
    m.request_action( my_source, s );
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
    try {
        m.store_image( result );
    } catch ( const std::runtime_error& e ) {
        std::cerr << "Failed to store image: " << e.what() << std::endl;
    }
}

void Manager::Source::operator()( const Close&, Manager& ) {
    boost::lock_guard<boost::mutex> lock( handler_mutex );
    if ( handler )
        handler->notice_closed_data_window();
}

void Manager::Source::operator()( const SetLimit& i, Manager& ) {
    std::cerr << "Noticing key limits for key " << i.key << " " << i.lower_limit << " " << i.limit << std::endl;
    boost::lock_guard<boost::mutex> lock( handler_mutex );
    if ( handler )
        handler->notice_user_key_limits( i.key, i.lower_limit, i.limit );
}

void Manager::Source::operator()( const DrawRectangle& i, Manager& ) {
    boost::lock_guard<boost::mutex> lock( handler_mutex );
    if ( handler )
        handler->notice_drawn_rectangle( i.l, i.r, i.t, i.b );
}

void Manager::Source::operator()(const Disassociation& d, Manager& m ) {
    print_status("Destructing ", true);
    boost::lock_guard< boost::mutex > lock( m.source_list_mutex );
    m.sources_queue.erase( std::remove( m.sources_queue.begin(), m.sources_queue.end(), shared_from_this() ) );
}

void Manager::request_action( boost::shared_ptr<Source> on, const Request& request ) {
    if ( boost::this_thread::get_id() == ui_thread.get_id() ) {
        boost::apply_visitor( boost::bind( boost::ref(*on), _1, boost::ref(*this) ), request );
    } else {
        boost::lock_guard<boost::mutex> lock(request_mutex);
        requests.push_back( std::make_pair( on, request ) );
    }
}

std::auto_ptr< dStorm::display::Manager > make_test_plugin_manager( dStorm::display::Manager* old )
{
    return std::auto_ptr< dStorm::display::Manager >( new Manager(old) );
}

boost::shared_ptr<Manager::Source> Manager::next_source() {
    boost::lock_guard< boost::mutex > lock( source_list_mutex );
    if ( sources_queue.empty() ) return boost::shared_ptr<Source>();
    boost::shared_ptr<Source> s = sources_queue.front();
    sources_queue.erase( sources_queue.begin() );
    sources_queue.push_back( s );
    return s;
}

