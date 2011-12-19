#ifndef DSTORM_CONFIG_H
#define DSTORM_CONFIG_H

#include <dStorm/input/chain/Link_decl.h>
#include <dStorm/engine/SpotFinder_decl.h>
#include <dStorm/engine/SpotFitterFactory_decl.h>
#include "InsertionPlace.h"
#include <memory>

namespace dStorm {
namespace output { class OutputSource; }

struct Config 
{
    virtual ~Config() {}
    virtual void add_input( std::auto_ptr<input::chain::Link>, InsertionPlace ) = 0;
    void add_input( input::chain::Link* s, InsertionPlace p ) 
        { add_input( std::auto_ptr<input::chain::Link>(s), p ); }
    virtual void add_spot_finder( std::auto_ptr<engine::spot_finder::Factory> ) = 0;
    void add_spot_finder( engine::spot_finder::Factory* f ) 
            { add_spot_finder( std::auto_ptr<engine::spot_finder::Factory>(f) ); }
    virtual void add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory> ) = 0;
    void add_spot_fitter( engine::spot_fitter::Factory* f ) 
            { add_spot_fitter( std::auto_ptr<engine::spot_fitter::Factory>(f) ); }
    virtual void add_output( std::auto_ptr<output::OutputSource> ) = 0;
    void add_output( output::OutputSource* s ) 
        { add_output( std::auto_ptr<output::OutputSource>(s) ); }
};

}

#endif
