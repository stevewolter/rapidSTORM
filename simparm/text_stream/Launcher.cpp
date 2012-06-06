#include "Launcher.h"
#include "InputStream.h"
#include <boost/thread/thread.hpp>

namespace simparm {
namespace text_stream {

Launcher::Launcher
    ( dStorm::job::Config& c, dStorm::MainThread& main_thread )
: trigger("TwiddlerControl", 
                "Read stdin/out for simparm control commands"),
  config(c),
  main_thread(main_thread)
{
}

void Launcher::attach_ui( simparm::NodeHandle n )
{
    trigger.attach_ui( n );
    listening = trigger.value.notify_on_value_change( 
        boost::bind( &Launcher::run_twiddler, this ) );
}

Launcher::~Launcher() {}

void Launcher::run_twiddler() {
    boost::shared_ptr< dStorm::InputStream > input_stream = dStorm::InputStream::create( main_thread, config );
    boost::thread thread( &dStorm::InputStream::processCommands, input_stream );
    thread.detach();
}

}
}
