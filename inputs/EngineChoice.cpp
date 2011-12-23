#ifndef DSTORM_CONFIG_ENGINE_CHOICE_H
#define DSTORM_CONFIG_ENGINE_CHOICE_H

#include "EngineChoice.h"
#include <dStorm/input/chain/Alternatives.h>
#include <dStorm/input/InputMutex.h>

namespace dStorm {

class EngineChoice
: public input::chain::Alternatives
{
  public:
    EngineChoice() 
        : Alternatives("Engine", "Choose engine", true) {}

    EngineChoice* clone() const { return new EngineChoice(*this); }
    std::string name() const { return "EngineChoice"; }

    void insert_new_node( std::auto_ptr<Link> link, Place p ) {
        if ( p == AsEngine )
            add_choice( link );
        else
            Alternatives::insert_new_node(link,p);
    }
};

std::auto_ptr< input::chain::Link > make_engine_choice()
    { return std::auto_ptr< input::chain::Link >( new EngineChoice() ); }

}

#endif
