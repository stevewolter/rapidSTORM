#include "Launcher.h"
#include "InputStream.h"
#include <boost/thread/thread.hpp>

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

void Launcher::run_twiddler() {
    boost::shared_ptr< dStorm::InputStream > input_stream = dStorm::InputStream::create( config, wxWidgets );
    boost::thread thread( &dStorm::InputStream::processCommands, input_stream );
    thread.detach();
}

}
}
