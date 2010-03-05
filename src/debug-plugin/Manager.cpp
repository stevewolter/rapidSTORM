#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "Manager.h"
#include "md5.h"
#include <iomanip>
#include <sstream>

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
            Source(properties, source) );
    }
    ~Handle() {
        guard lock(m.mutex);
        m.sources.erase( i );
        std::cerr << "Handle destroyed\n";
    }

    std::auto_ptr<dStorm::Display::Change>
        get_state();
};

Manager::Source::Source(
    const WindowProperties& properties,
    dStorm::Display::DataSource& source )
: handler(source)
{
    std::cerr << "Listening to window " 
              << properties.name << "\n";
    handle_resize( properties.initial_size );
}

void Manager::Source::handle_resize( 
    const dStorm::Display::ResizeChange& r)
{
    std::cerr << "Sizing display to " 
              << r.width << " " << r.height << " with "
              << r.key_size << " grey levels and pixel "
                 "size " << r.pixel_size << "\n";
    current_display = Image::Zero( r.width, r.height );
    state.do_resize = true;
    state.resize_image = r;
    state.change_key.resize(
        state.resize_image.key_size );
}

bool Manager::Source::get_and_handle_change() {
    std::auto_ptr<dStorm::Display::Change> c
        = handler.get_changes();

    if ( c.get() == NULL ) {
        std::cerr << "Error in display handling: NULL change pointer\n";
        return false;
    }

    bool has_changed = c->do_resize | c->do_clear |
                       c->do_change_image | 
                       (c->change_pixels.size() > 0) |
                       (c->change_key.size() > 0);
    if ( c->do_resize ) {
        handle_resize( c->resize_image );
    } else if ( c->do_clear ) {
        state.do_clear = true;
        state.clear_image = c->clear_image;
        current_display.fill(
            state.clear_image.background );
    } else if ( c->do_change_image ) {
        for (int x = 0; x < current_display.rows(); x++)
          for (int y = 0; y < current_display.cols(); y++)
          {
            int i = y*current_display.cols()+x;
            current_display(x,y) = 
                c->image_change.pixels[i];
          }
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
            thread = boost::thread(
                boost::bind( 
                    &Manager::dispatch_events,
                    this )
            );
        }
    }
    return 
        std::auto_ptr<dStorm::Display::Manager::WindowHandle>( new Handle(props, handler, *this) );
}

std::ostream& operator<<(std::ostream& o, dStorm::Display::Color c)
{
    return ( o << int(c.red()) << " " << int(c.green()) << " " << int(c.blue()) );
}

void Manager::dispatch_events() {
    std::cerr << "Start of event dispatch routine\n";
    while ( running ) {
      {
        guard lock(mutex);
        gui_run.signal();
        for ( Sources::iterator i = sources.begin(); i != sources.end(); i++ )
        {
            bool has_changed = i->get_and_handle_change();
            if (!has_changed) continue;
            
            md5_state_t pms;
            md5_byte_t digest[16];
            md5_init( &pms );
            md5_append( &pms, 
                (md5_byte_t*)i->current_display.data(),
                i->current_display.size()*
                    sizeof(dStorm::Pixel) / sizeof(md5_byte_t));
            md5_finish( &pms, digest );

            std::stringstream sdigest;
            for (int j = 0; j < 16; j++)
                sdigest << std::hex << int(digest[j]);
            std::cerr << "After change digest is " << sdigest.str() << "\n";
        }
      }
#ifdef HAVE_USLEEP
      usleep(10000);
#elif HAVE_WINDOWS_H
      Sleep(10);
#endif
    }
    std::cerr << "End of event dispatch routine\n";
}

void Manager::run_in_GUI_thread( ost::Runnable* )
{
}

Manager::Manager(dStorm::Display::Manager *p)
: gui_run(mutex),
  running(false),
  previous(p)
{
}

Manager::~Manager()
{
    if ( running ) {
        running = false;
        thread.join();
    }
}

void Manager::store_image(
        std::string filename,
        const dStorm::Display::Change& image )
{
    std::cerr << "Storing under " << filename << std::endl;
    previous->store_image(filename, image);
    std::cerr << "Stored" << std::endl;
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
    int w = rv->resize_image.width,
        h = rv->resize_image.height;
    assert( i->current_display.rows() == w );
    assert( i->current_display.cols() == h );
    rv->do_change_image = true;
    rv->image_change.pixels.resize( w*h );
    dStorm::Display::Color *p = 
        rv->image_change.pixels.ptr();
    for (int y = 0; y < h; y++)
      for (int x = 0; x < w; x++)
      {
        *p = i->current_display(x,y);
        ++p;
      }

    return rv;
}

