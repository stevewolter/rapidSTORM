#include "Manager.h"
#include "md5.h"

class Manager::Handle 
: public dStorm::Display::Manager::WindowHandle {
    dStorm::Display::DataSource& source;
    Manager& m;

  public:
    Handle( 
        dStorm::Display::DataSource& source,
        Manager& m ) : source(source), m(m) 
    {
        guard lock(m.mutex);
        m.sources.insert( & source );
    }
    ~Handle() {
        guard lock(m.mutex);
        m.sources.erase( & source );
        std::cerr << "Handle destroyed\n";
    }
};

std::auto_ptr<dStorm::Display::Manager::WindowHandle>
    Manager::register_data_source
(
    const WindowProperties& ,
    dStorm::Display::DataSource& handler
) {
    {
        guard lock(mutex);
        if ( thread == boost::thread() ) {
            thread = boost::thread(
                boost::bind( 
                    &Manager::dispatch_events,
                    this )
            );
        }
    }
    std::cerr << "Made handle\n";
    return 
        std::auto_ptr<dStorm::Display::Manager::WindowHandle>( new Handle(handler, *this) );
}

void Manager::dispatch_events() {
    std::cerr << "Dispatching events\n";
    guard lock(mutex);
    std::cerr << "Got event lock\n";
    while ( ! sources.empty() )
        for ( Sources::iterator i = sources.begin(); i != sources.end(); i++ )
        {
            std::cerr << "Dispatching source\n";
        }
}

void Manager::run_in_GUI_thread( ost::Runnable* )
{
}

Manager::Manager()
: was_started(false)
{
}
