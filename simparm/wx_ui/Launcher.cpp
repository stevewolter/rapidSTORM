/* wxWidgets must be included first in Windows due to conflicts with Boost's
 * handling of windows.h. */
#include <wx/filename.h>
#include "Launcher.h"
#include "RootNode.h"

namespace simparm {
namespace wx_ui {

Launcher::Launcher
    ( dStorm::job::Config& c )
: trigger("wxControl", "Show wxWidgets user interface"),
  config(c)
{
}

void Launcher::attach_ui( simparm::NodeHandle n )
{
    trigger.attach_ui( n );
    listening = trigger.value.notify_on_value_change( 
        boost::bind( &Launcher::was_triggered, this ) );
}

Launcher::~Launcher() {}

void Launcher::was_triggered() {
    if ( trigger.triggered() ) {
        trigger.untrigger();
        launch();
    }
}

void Launcher::launch() {
    wxString tempFile = wxFileName::CreateTempFileName( wxT("rapidstorm-log") );
    RootNode::create( config, std::string( tempFile.mb_str() ) );
}

}
}
