#include "ShiftSpeedControl.h"
#include "SDK.h"
#include <string.h>
#include <stdio.h>
#include "Config.h"
#include "System.h"
#include <math.h>
#include <simparm/ChoiceEntry_Impl.hh>

using namespace simparm;
using namespace SDK;

namespace AndorCamera {

using namespace States;
using namespace Phases;

_ShiftSpeedControl::_ShiftSpeedControl() :
  adChannelDepth("DesiredADChannelDepth", "Desired AD Channel Depth", 14),
  adChannel("ADChannel", "AD Channel"),
  desired_VS_Speed("DesiredVSSpeed",
                   "Desired vertical shift time (µs)",3.4),
  desired_HS_Speed("DesiredHSSpeed",
                   "Desired horizontal shift speed (MHz)", 10)
{
    adChannel.setUserLevel(Entry::Intermediate);
    desired_VS_Speed.setUserLevel(Entry::Expert);
    desired_HS_Speed.setUserLevel(Entry::Expert);
    adChannelDepth.setUserLevel(Entry::Expert);
}

ShiftSpeedControl::ShiftSpeedControl(StateMachine& sm, Config &conf)
: Object("ShiftSpeedControl", "ShiftSpeedControl"),
  sm(sm),
  outputAmp( conf.outputAmp ),
  VS_Speed( conf.VS_Speed ),
  HS_Speed( conf.HS_Speed )
{
    registerNamedEntries();
}
  
ShiftSpeedControl::ShiftSpeedControl(const ShiftSpeedControl&c)
: Node(c), Object(c),
  _ShiftSpeedControl(c),
  Listener(),
  Node::Callback(),
  sm(c.sm),
  outputAmp( c.outputAmp ),
  VS_Speed( c.VS_Speed ),
  HS_Speed( c.HS_Speed )
{
    registerNamedEntries();
}

static void standard_show_policy( StateMachine &sm, simparm::Entry &e ) 
{
    sm.add_managed_attribute( e.viewable, Initialized, Acquiring );
}
static void pre_init_show_policy( StateMachine &sm, simparm::Entry &e ) 
{
    sm.add_managed_attribute( e.viewable, Disconnected );
    sm.add_managed_attribute( e.editable, Disconnected );
}

static void remove_both_attributes( StateMachine &sm, simparm::Entry &e )
{
    sm.remove_managed_attribute( e.viewable );
    sm.remove_managed_attribute( e.editable );
}

ShiftSpeedControl::~ShiftSpeedControl() 
{
    remove_both_attributes( sm, adChannelDepth );
    sm.remove_managed_attribute( adChannel.viewable );
    remove_both_attributes( sm, desired_VS_Speed );
    sm.remove_managed_attribute( VS_Speed.viewable );
    remove_both_attributes( sm, desired_HS_Speed );
    sm.remove_managed_attribute( HS_Speed.viewable );
}

void ShiftSpeedControl::registerNamedEntries() 
{
    /* We need connection to the adChannel to dynamically adjust the
     * HS_Speed values. */
    receive_changes_from( adChannel.value );

    pre_init_show_policy( sm, adChannelDepth );
    standard_show_policy( sm, adChannel );
    pre_init_show_policy( sm, desired_VS_Speed );
    standard_show_policy( sm, VS_Speed );
    pre_init_show_policy( sm, desired_HS_Speed );
    standard_show_policy( sm, HS_Speed );

    push_back( adChannelDepth );
    push_back( adChannel );
    push_back( desired_VS_Speed );
    push_back( VS_Speed );
    push_back( desired_HS_Speed );
    push_back( HS_Speed );
}

void ShiftSpeedControl::operator()(Node &src, 
    Node::Callback::Cause c, Node *)
 
{
    try {
        if (&src == &adChannel.value && c == ValueChanged) {
            fillHSSpeed();
        } 
    } catch (const std::exception&e ) {
        std::cerr << e.what() << "\n";
    }
}

void ShiftSpeedControl::controlStateChanged(
    Phase phase, State from, State to
)
{
    if ( phase == Review && 
         from == lower_state(Initialized) && to == Initialized) 
    {
        fillADChannel();
        fillVSSpeed();
    } else if ( phase == Transition && to == Acquiring ) {
        adChannel.editable = false;
        if (adChannel.isValid())
            SDK::SetADChannel( adChannel() );
        HS_Speed.editable = false;
        if (HS_Speed.isValid())
            SDK::SetHSSpeed( outputAmp(), HS_Speed() );
        VS_Speed.editable = false;
        if (VS_Speed.isValid())
            SDK::SetVSSpeed( VS_Speed() );
    } else if ( phase == Transition && to == Acquiring ) {
        adChannel.editable = true;
        HS_Speed.editable = true;
        VS_Speed.editable = true;
    }
}

/* See header file for documentation */
void ShiftSpeedControl::fillADChannel() {
    int numADChannels = SDK::GetNumberADChannels(),
        preferredADChannel = 0;

    adChannel.removeAllChoices();

    for (int i = 0; i < numADChannels; i++) {
        int bitDepth = GetBitDepth( i );

        char buffer[20];
        sprintf(buffer, "%i bits", bitDepth);
        adChannel.DataChoiceEntry<int>::addChoice
            (i, string(1, 'A'+i), string(buffer));
        
        if (bitDepth == (int)adChannelDepth())
            preferredADChannel = i;
    }
    adChannel = preferredADChannel;
    adChannel.setViewable( numADChannels > 1 );
}

/* See header file for documentation */
void ShiftSpeedControl::fillVSSpeed() {
    int numVSSpeeds = SDK::GetNumberVSSpeeds(), preferredVSSpeed = -1;

    VS_Speed.removeAllChoices();

    float vals[numVSSpeeds];
    for (int i = 0; i < numVSSpeeds; i++) {
        vals[i] = GetVSSpeed(i);
        char buffer[20];
        sprintf(buffer, "%g µs", vals[i]);
        VS_Speed.addChoice(i, string(1, 'A'+i), string(buffer));
        
        if ( preferredVSSpeed == -1 ||
                  fabs(vals[preferredVSSpeed] - desired_VS_Speed())
                > fabs(vals[i] - desired_VS_Speed()) ) 
            preferredVSSpeed = i;
    }
    VS_Speed = preferredVSSpeed;
    VS_Speed.setViewable( numVSSpeeds > 1 );
}

/* See header file for documentation */
void ShiftSpeedControl::fillHSSpeed() {
    if ( ! adChannel.isValid() ) {
        HS_Speed.removeAllChoices();
    } else {
        OutputAmp outputAmp = this->outputAmp();
        int adChannel = this->adChannel.value()();
        int numHSSpeeds, preferredHSSpeed = -1;
        simparm::DataChoice<int>* preferredChoice = NULL;

        std::string old_choice = 
            (HS_Speed.isValid() ? HS_Speed.value().getName() : "" );

        numHSSpeeds = SDK::GetNumberHSSpeeds(adChannel, outputAmp);
        HS_Speed.removeAllChoices();

        float vals[numHSSpeeds];
        for (int i = 0; i < numHSSpeeds; i++) {
            vals[i] = SDK::GetHSSpeed(adChannel, (OutputAmp)outputAmp, i);
            char name[20], desc[20];
            sprintf(name, "%g", vals[i]);
            sprintf(desc, "%g MHz", vals[i]);
            HS_Speed.addChoice(i, string(name), string(desc));
            
            if ( preferredHSSpeed == -1 ||
                      fabs(vals[preferredHSSpeed] - desired_HS_Speed())
                    > fabs(vals[i] - desired_HS_Speed()) ) 
            {
                preferredHSSpeed = i;
                preferredChoice = &HS_Speed[string(name)];
            }
        }
        if ( old_choice != "" && HS_Speed.hasChoice( old_choice ) )
            HS_Speed.choose( old_choice );
        else if ( preferredChoice != NULL )
            HS_Speed.value = preferredChoice;

        HS_Speed.setViewable( numHSSpeeds > 1 );
    }
}

}
