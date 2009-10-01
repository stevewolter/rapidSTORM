#include "Initialization.h"
#include "SDK.h"
#include <string.h>

using namespace simparm;

namespace AndorCamera {

_Initialization::_Initialization()
  /* Assume MS windows path structure */
: configDir("CameraInitDir","Location of camera's DETECTOR.INI",
            "..\\share\\andor"),
  connect("ConnectToCamera","Connect to camera"),
  disconnect("DisconnectFromCamera","Shut down camera")
{
    configDir.setHelp("Some Andor cameras must be initialized with "
                      "a DETECTOR.INI file and a .sys driver file "
                      "that define the camera's capabilities.");
    configDir.setUserLevel(Entry::Expert);
}

void Initialization::registerNamedEntries() 
{
    receive_changes_from( connect.value );
    receive_changes_from( disconnect.value );

    sm.add_managed_attribute( connect.viewable, States::Disconnected );
    sm.add_managed_attribute( connect.editable, States::Disconnected );
    sm.add_managed_attribute( configDir.viewable, States::Disconnected );
    sm.add_managed_attribute( configDir.editable, States::Disconnected );
    sm.add_managed_attribute( disconnect.viewable, States::Initialized );
    sm.add_managed_attribute( disconnect.editable, States::Initialized );

    push_back( configDir );
    push_back( connect );
    push_back( disconnect );
}

Initialization::Initialization(StateMachine& sm)
: Object("Initialization", "Initialization"),
  sm(sm)
{
    registerNamedEntries();
}
  
Initialization::Initialization(const Initialization&c)
: Node(c),
  Object(c),
  _Initialization(c),
  StateMachine::Listener(),
  Node::Callback(),
  sm(c.sm)
{
    registerNamedEntries();
}

Initialization::~Initialization() 
{
    sm.remove_managed_attribute( configDir.viewable );
    sm.remove_managed_attribute( configDir.editable );
    sm.remove_managed_attribute( connect.viewable );
    sm.remove_managed_attribute( connect.editable );
    sm.remove_managed_attribute( disconnect.viewable );
    sm.remove_managed_attribute( disconnect.editable );
}

void Initialization::operator()(Node &src, 
    Node::Callback::Cause c, Node *)
 
{
    try {
        if ( c == ValueChanged && &src == &connect.value 
            && connect.triggered() ) 
        {
            connect.untrigger();
            sm.ensure_at_least(States::Initialized);
        } else if ( c == ValueChanged && &src == &disconnect.value 
            && disconnect.triggered() )
        {
            disconnect.untrigger();
            sm.ensure_at_most(States::Disconnected);
        }
    } catch (const std::exception&e ) {
        std::cerr << e.what() << "\n";
    }
}

using namespace States;
using namespace Phases;

void Initialization::controlStateChanged( Phase phase, State from, State to
)
{
    STATUS("Initialization for phase " << phase << " of " << from << " to "
           << to);
    if ( phase == Transition && from == Disconnected &&
                                to == Initialized ) 
    {
        sm.status = "Connecting to camera"; 
        SDK::Initialize( configDir() );
        sm.status = "Connected to camera";
    } else if ( phase == Transition && to == Disconnected &&
                                     from == Initialized ) 
    {
        SDK::ShutDown();
    }
}

}
