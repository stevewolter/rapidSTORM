#include "ShiftSpeedControl.h"
#include "SDK.h"
#include <string.h>
#include <stdio.h>
#include "Config.h"
#include "System.h"
#include <math.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include "StateMachine_impl.h"
#include <simparm/EntryManipulators.hh>

#include <boost/units/io.hpp>
#include <boost/units/systems/si/io.hpp>
#include <boost/units/cmath.hpp>

using namespace simparm;
using namespace boost::units;
using namespace SDK;

namespace AndorCamera {

_ShiftSpeedControl::_ShiftSpeedControl() :
  adChannelDepth("DesiredADChannelDepth", "Desired AD Channel Depth", 14),
  adChannel("ADChannel", "AD Channel"),
  desired_VS_Speed("DesiredVSSpeed", "Desired vertical shift time"),
  desired_HS_Speed(
    "DesiredHSSpeed",
    "Desired horizontal shift speed")
{
    desired_HS_Speed = quantity< si::megafrequency, float >(10E6f * si::hertz);
    desired_VS_Speed = quantity< si::microtime, float >( 3.4 * si::microsecond );
    adChannel.setUserLevel(Object::Intermediate);
    desired_VS_Speed.setUserLevel(Object::Expert);
    desired_HS_Speed.setUserLevel(Object::Expert);
    adChannelDepth.setUserLevel(Object::Expert);
}

ShiftSpeedControl::ShiftSpeedControl(StateMachine& sm, Config &conf)
: Object("ShiftSpeedControl", "ShiftSpeedControl"),
  StateMachine::StandardListener<ShiftSpeedControl>(*this),
  Node::Callback(simparm::Event::ValueChanged),
  sm(sm),
  outputAmp( conf.outputAmp ),
  VS_Speed( conf.VS_Speed ),
  HS_Speed( conf.HS_Speed )
{
    registerNamedEntries();
}
  
ShiftSpeedControl::ShiftSpeedControl(const ShiftSpeedControl&c)
: Object(c),
  _ShiftSpeedControl(c),
  StateMachine::StandardListener<ShiftSpeedControl>(*this),
  Node::Callback(simparm::Event::ValueChanged),
  sm(c.sm),
  outputAmp( c.outputAmp ),
  VS_Speed( c.VS_Speed ),
  HS_Speed( c.HS_Speed )
{
    registerNamedEntries();
}

ShiftSpeedControl::~ShiftSpeedControl() 
{
}

void ShiftSpeedControl::registerNamedEntries() 
{
    /* We need connection to the adChannel to dynamically adjust the
     * HS_Speed values. */
    receive_changes_from( adChannel.value );

    push_back( adChannelDepth );
    push_back( adChannel );
    push_back( desired_VS_Speed );
    push_back( VS_Speed );
    push_back( desired_HS_Speed );
    push_back( HS_Speed );
}

void ShiftSpeedControl::operator()(const simparm::Event& e)
{
    try {
        if (&e.source == &adChannel.value) {
            fillHSSpeed();
        } 
    } catch (const std::exception&e ) {
        std::cerr << "Error while reading horizontal shift speeds: " << e.what() << "\n";
    }
}

MK_EMPTY_RW(ShiftSpeedControl)

template <>
class ShiftSpeedControl::Token<States::Connected>
: public States::Token
{
    simparm::UsabilityChanger dvs, dhs, acd, ac, vs, hs;
  public:
    Token( ShiftSpeedControl& s )
    : dvs(s.desired_VS_Speed, false),
      dhs(s.desired_HS_Speed, false),
      acd(s.adChannelDepth, false),
      ac(s.adChannel, true),
      vs(s.VS_Speed, true),
      hs(s.HS_Speed, true)
    {
        s.fillADChannel();
        s.fillVSSpeed();
    }
};

class ShiftSpeedControl::ManagedAcquisition {
    simparm::EditabilityChanger ac, hs, vs;

  public:
    ManagedAcquisition( ShiftSpeedControl& s ) 
      : ac( s.adChannel, false ),
        hs( s.HS_Speed, false ),
        vs( s.VS_Speed, false ) 
    {
        if (s.adChannel.isValid())
            SDK::SetADChannel( s.adChannel() );
        if (s.HS_Speed.isValid())
            SDK::SetHSSpeed( s.outputAmp(), s.HS_Speed() );
        if (s.VS_Speed.isValid())
            SDK::SetVSSpeed( s.VS_Speed() );
    }
};

template <>
class ShiftSpeedControl::Token<States::Readying> 
: public States::Token
{
    std::auto_ptr<ManagedAcquisition> a;
  public:
    Token(ShiftSpeedControl& s) : a( new ManagedAcquisition(s) ) {}
};

template class StateMachine::StandardListener<ShiftSpeedControl>;

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

    quantity<si::microtime> vals[numVSSpeeds];
    for (int i = 0; i < numVSSpeeds; i++) {
        vals[i] = GetVSSpeed(i) * si::microsecond;
        std::stringstream desc;
        desc << vals[i];
        VS_Speed.addChoice(i, string(1, 'A'+i), desc.str());
        
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

        quantity<si::megafrequency> vals[numHSSpeeds];
        for (int i = 0; i < numHSSpeeds; i++) {
            vals[i] = SDK::GetHSSpeed(adChannel, (OutputAmp)outputAmp, i)
                        * si::megahertz;
            std::stringstream name, desc;
            name << vals[i].value();
            desc << vals[i];
            HS_Speed.addChoice(i, name.str(), desc.str());
            
            if ( preferredHSSpeed == -1 ||
                      fabs(vals[preferredHSSpeed] - desired_HS_Speed())
                    > fabs(vals[i] - desired_HS_Speed()) ) 
            {
                preferredHSSpeed = i;
                preferredChoice = &HS_Speed[name.str()];
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
