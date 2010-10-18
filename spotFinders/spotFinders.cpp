//#define VERBOSE
#include "debug.h"
#define SIMPARM_DEBUG(x) DEBUG(x)
#include "spotFinders.h"
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/Config.h>
#include <simparm/ChoiceEntry_Impl.hh>

#include "Spalttiefpass.h"
#include "MedianSmoother.h"
#include "ErosionSmoother.h"
#include "GaussSmoothing.h"

namespace dStorm {
namespace spotFinders {

void basic_spotFinders( engine::Config &to_this_config )
{
    simparm::NodeChoiceEntry<engine::SpotFinderFactory>& c = 
        to_this_config.spotFindingMethod;
    c.addChoice( new Spalttiefpass::Factory() );
    c.addChoice( new MedianSmoother::Factory() );
    c.addChoice( new ErosionSmoother::Factory() );
    c.addChoice( new GaussSmoother::Factory() );
}

}
}