#include "Initialization.h"
#include "SDK.h"
#include <string.h>
#include <simparm/EntryManipulators.hh>
#include "StateMachine_impl.h"
#include "System.h"

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

    disconnect.viewable = disconnect.editable = false;
    configDir.viewable = configDir.editable = false;
}

void Initialization::registerNamedEntries() 
{
    receive_changes_from( connect.value );
    receive_changes_from( disconnect.value );

    push_back( configDir );
    push_back( connect );
    push_back( disconnect );
}

Initialization::Initialization(StateMachine& sm)
: Object("Initialization", "Initialization"),
  StateMachine::StandardListener<Initialization>(*this),
  simparm::Node::Callback( simparm::Event::ValueChanged ),
  sm(sm)
{
    registerNamedEntries();
}
  
Initialization::Initialization(const Initialization&c)
: Object(c),
  _Initialization(c),
  StateMachine::StandardListener<Initialization>(*this),
  Node::Callback( simparm::Event::ValueChanged ),
  sm(c.sm)
{
    registerNamedEntries();
}

Initialization::~Initialization() 
{
}

void Initialization::operator()
    (const simparm::Event& e)
 
{
    if ( &e.source == &connect.value && connect.triggered() ) 
    {
        connect.untrigger();
        sm.ensure_at_least(States::Connected);
    } else if ( &e.source == &disconnect.value && disconnect.triggered() )
    {
        disconnect.untrigger();
        sm.ensure_at_most(States::Disconnected);
    }
}

using namespace States;

MK_EMPTY_RW(Initialization)

template <>
class Initialization::Token<Connecting>
: public States::Token
{
    Initialization& i;
    simparm::AttributeChange<bool> cam_chooser;
    simparm::UsabilityChanger connect, configDir;

  public:
    Token(Initialization& i) 
        : i(i),
          cam_chooser( System::singleton().get_camera_chooser().editable, 
                       false ),
          connect( i.connect, false ), configDir( i.configDir, false ) 
    {
        if ( i.is_active ) {
            i.sm.status = "Connecting to camera"; 
            SDK::Initialize( i.configDir() );
            i.sm.status = "Connected to camera";
        }
    }

    ~Token() {
        std::cerr << "Called " << i.is_active << std::endl;
        if ( i.is_active )
            SDK::ShutDown();
    }
};

template <>
class Initialization::Token<Connected>
: public States::Token
{
    simparm::UsabilityChanger disconnect;

  public:
    Token(Initialization& i) : disconnect(i.disconnect, true) {}
};

template class StateMachine::StandardListener<Initialization>;

}
