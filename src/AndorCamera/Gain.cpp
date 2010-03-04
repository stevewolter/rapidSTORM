#include "Gain.h"
#include "SDK.h"
#include "StateMachine_impl.h"
#include <simparm/EntryManipulators.hh>

namespace AndorCamera {

Gain::Gain(StateMachine& sm, Config &config) 
: simparm::Object("Gain", "Trigger options"), 
  StateMachine::StandardListener<Gain>(*this),
  sm(sm), emccdGain(config.emccdGain) 
{ 
    emccdGain = 200; 
    push_back( emccdGain );
}

MK_EMPTY_RW(Gain)

template<>
Gain::Token<States::Connected>::Token( Gain& g ) 
: parent(g)
{
    std::pair<int,int> gainRange = SDK::GetEMCCDGainRange();
    g.emccdGain.setMin( gainRange.first );
    g.emccdGain.setMax( gainRange.second );
}

template <>
class Gain::Token<States::Readying>
: public States::Token
{
    simparm::EditabilityChanger e;
  public:
    Token(Gain& g) : e(g.emccdGain, !g.is_active) {
        SDK::SetEMCCDGain( g.emccdGain() );
    }
};

template class StateMachine::StandardListener<Gain>;

}
