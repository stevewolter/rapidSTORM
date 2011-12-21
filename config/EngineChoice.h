#ifndef DSTORM_CONFIG_ENGINECHOICE_H
#define DSTORM_CONFIG_ENGINECHOICE_H

#include <dStorm/input/chain/Link.h>
#include <dStorm/engine/SpotFinder_decl.h>
#include <dStorm/engine/SpotFitterFactory_decl.h>

namespace dStorm {

class GrandConfig;

struct IEngineChoice 
: public input::chain::Link
{
    virtual void add( std::auto_ptr<engine::spot_finder::Factory> s ) = 0;
    virtual void add( std::auto_ptr<engine::spot_fitter::Factory> s ) = 0;
};

std::auto_ptr< IEngineChoice > make_engine_choice(GrandConfig&);
std::auto_ptr< IEngineChoice > copy_engine_choice(const IEngineChoice&, GrandConfig&);

}

#endif
