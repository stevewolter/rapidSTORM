#include "Launcher.h"
#include "RootNode.h"

namespace simparm {
namespace wx_ui {

Launcher::Launcher
    ( dStorm::job::Config& c, dStorm::MainThread& main_thread )
: simparm::TriggerEntry("wxControl", "Show wxWidgets user interface"),
  config(c),
  main_thread(main_thread)
{
}

void Launcher::attach_ui( simparm::NodeHandle n )
{
    simparm::TriggerEntry::attach_ui( n );
    listening = value.notify_on_value_change( 
        boost::bind( &Launcher::launch, this ) );
}

Launcher::~Launcher() {}

void Launcher::launch() {
    if ( triggered() ) {
        untrigger();
        RootNode::create( main_thread, config );
    }
}

}
}
