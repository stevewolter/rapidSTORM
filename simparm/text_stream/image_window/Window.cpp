#include "Window.h"
#include "MainThread.h"

#include <simparm/text_stream/Node.h>
#include <simparm/text_stream/InnerBackendNode.h>
#include <boost/lexical_cast.hpp>
#include <dStorm/display/store_image.h>
#include "md5.h"

namespace simparm {
namespace text_stream {
namespace image_window {

class Window::GUINode : public simparm::text_stream::Node {
    Window& c;
    struct Backend : public simparm::text_stream::InnerBackendNode {
        Window& c;
        void process_command_( const std::string&, std::istream& );
        Backend( GUINode& g, boost::shared_ptr<BackendNode> parent )
            : InnerBackendNode("PixelQuery", "Object", g, parent ), c(g.c) {}
        ~Backend() {}
    };
public:
    GUINode( Window& c, simparm::NodeHandle parent ) 
    : simparm::text_stream::Node("PixelQuery", "Object"), c(c) {
        simparm::text_stream::Node* p = dynamic_cast< simparm::text_stream::Node* >(parent.get());
        if ( p )
            set_backend_node( std::auto_ptr<simparm::text_stream::BackendNode>(
                new Backend( *this, p->get_backend() ) ) );
    }
};

void Window::GUINode::Backend::process_command_( const std::string& command, std::istream& in )
{
    if ( command == "pixel_value" ) {
        std::string window;
        dStorm::display::Image::Position pos;
        boost::shared_ptr<Window> i = c.shared_from_this();
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

Window::Window(
    MainThread& m,
    const dStorm::display::WindowProperties& properties,
    boost::shared_ptr<dStorm::display::DataSource> source,
    int n)
: m(m), handler(source), state(0), number(n),
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
  close_("Close", "Close Window"),
  set_lower_limit("SetLowerLimit", "Set lower key limit"),
  set_upper_limit("SetUpperLimit", "Set upper key limit"),
  draw_rectangle_("DrawRectangle", "Draw rectangle")
{
    handle_resize( properties.initial_size );
}

Window::~Window() {}

void Window::attach_ui( simparm::NodeHandle at ) {
    listening[0] = close_.value.notify_on_value_change( 
        boost::bind( &Window::close_window, this ) );
    listening[1] = set_lower_limit.value.notify_on_value_change( 
        boost::bind( &Window::notice_lower_limit, this ) );
    listening[2] = set_upper_limit.value.notify_on_value_change(
        boost::bind( &Window::notice_upper_limit, this ) );
    listening[3] = draw_rectangle_.value.notify_on_value_change(
        boost::bind( &Window::notice_drawn_rectangle, this ) );

    simparm::NodeHandle r = window_object.attach_ui( at );
    close_.attach_ui( r );
    which_key.attach_ui( r );
    new_limit.attach_ui( r );
    set_lower_limit.attach_ui( r );
    set_upper_limit.attach_ui( r );
    top.attach_ui( r);
    bottom.attach_ui( r);
    left.attach_ui( r);
    right.attach_ui( r);
    draw_rectangle_.attach_ui( r);
    digest.attach_ui( r );
    mean.attach_ui( r );
    nonzero_count.attach_ui( r );
    frame_number.attach_ui( r );
    key_31.attach_ui( r );
    window_width.attach_ui( r );

    gui_node.reset( new GUINode( *this, r ) );
}


void Window::handle_resize( 
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

bool Window::get_and_handle_change() {
    std::auto_ptr<dStorm::display::Change> c = handler->get_changes();
    if ( !c.get() ) { handle_disassociation(); return false; }

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

void Window::print_status()
{
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

void Window::save_window( const dStorm::display::SaveRequest& i ) {
    get_and_handle_change();

    assert( state.do_clear );

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
        store_image( result );
    } catch ( const std::runtime_error& e ) {
        std::cerr << "Failed to store image: " << e.what() << std::endl;
    }
}

void Window::close() 
{
    handler->notice_closed_data_window();
    handle_disassociation();
}

void Window::set_key_limit( int key, bool lower, std::string limit ) {
    handler->notice_user_key_limits( key, lower, limit );
}

void Window::draw_rectangle( int l, int r, int t, int b ) {
    handler->notice_drawn_rectangle( l, r, t, b );
}

void Window::handle_disassociation() {
    print_status();
    m.remove_window_from_event_queue( shared_from_this() );
}

void Window::close_window() {
    if ( close_.triggered() ) {
        close_.untrigger();
        m.request_action( boost::bind( &Window::close, shared_from_this() ) );
    }
}

void Window::notice_lower_limit() {
    if ( set_lower_limit.triggered() ) {
        set_lower_limit.untrigger();
        m.request_action( boost::bind( &Window::set_key_limit, shared_from_this(), which_key(), true, new_limit() ) );
    }
}

void Window::notice_upper_limit() {
    if ( set_upper_limit.triggered() ) {
        set_upper_limit.untrigger();
        m.request_action( boost::bind( &Window::set_key_limit, shared_from_this(), which_key(), false, new_limit() ) );
    }
}

void Window::notice_drawn_rectangle() {
    if ( draw_rectangle_.triggered() ) {
        draw_rectangle_.untrigger();
        m.request_action( boost::bind( &Window::draw_rectangle, shared_from_this(), left(), right(), top(), bottom() ) );
    }
}

void Window::update_window() {
    bool updated = get_and_handle_change();
    if ( updated ) print_status();
}

}
}
}
