#include "Launcher.h"
#include "InputStream.h"
#include <dStorm/GUIThread.h>
#include <boost/thread/thread.hpp>
#include <dStorm/stack_realign.h>

namespace simparm {
namespace text_stream {

Launcher::Launcher ( const dStorm::JobConfig& c, bool wxWidgets )
: trigger( (wxWidgets) ? "TwiddlerControl" : "SimparmControl", 
                "Read stdin/out for simparm control commands" + std::string( (wxWidgets) ? " and show image windows in wxWidgets GUI" : "") ),
  config(c),
  wxWidgets( wxWidgets )
{
}

void Launcher::attach_ui( simparm::NodeHandle n )
{
    trigger.attach_ui( n );
    listening = trigger.value.notify_on_value_change( 
        boost::bind( &Launcher::run_twiddler, this ) );
}

Launcher::~Launcher() {}

DSTORM_REALIGN_STACK static void run_input_stream( boost::shared_ptr<dStorm::InputStream> is )
{
    is->processCommands();
    is.reset();
    dStorm::GUIThread::get_singleton().join_this_thread();
}

void Launcher::run_twiddler() {
    boost::shared_ptr< dStorm::InputStream > input_stream = dStorm::InputStream::create( config, wxWidgets );
    std::auto_ptr<boost::thread> simparm_thread(
        new boost::thread(&run_input_stream, input_stream) );
    dStorm::GUIThread::get_singleton().wait_for_thread( simparm_thread );
}

}
}
