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
}

bool Manager::Source::get_and_handle_change() {
    std::auto_ptr<dStorm::Display::Change> c
        = handler.get_changes();

    if ( c.get() == NULL ) {
        std::cerr << "Error in display handling: NULL change pointer\n";
        return false;
    }

    std::cerr << "Handling change" << std::endl;
    bool has_changed = c->do_resize | c->do_clear |
                       c->do_change_image | 
                       (c->change_pixels.size() > 0) |
                       (c->change_key.size() > 0);
    if ( c->do_resize )
        handle_resize( c->resize_image );
    else if ( c->do_change_image ) {
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

    return has_changed;
}

std::auto_ptr<dStorm::Display::Manager::WindowHandle>
    Manager::register_data_source
(
    const WindowProperties& props,
    dStorm::Display::DataSource& handler
) {
    {
        std::cerr << "Acquiring mutex\n";
        guard lock(mutex);
        std::cerr << "Acquired mutex\n";
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
        for ( Sources::iterator i = sources.begin(); i != sources.end(); i++ )
        {
            bool has_changed = i->get_and_handle_change();
            if (!has_changed) continue;
            
            std::cerr << "Computing digest" << std::endl;
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

Manager::Manager()
: running(false)
{
}

Manager::~Manager()
{
    if ( running ) {
        running = false;
        thread.join();
    }
}
