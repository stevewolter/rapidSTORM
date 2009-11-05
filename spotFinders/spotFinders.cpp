#include "spotFinders.h"
#include <dStorm/engine/Config.h>
#include <dStorm/engine/SpotFinder.h>
#include <simparm/ChoiceEntry_Impl.hh>

#include "Spalttiefpass.h"
#include "MedianSmoother.h"
#include "ErosionSmoother.h"
#include "GaussSmoothing.h"

namespace dStorm {

void basic_spotFinders( dStorm::Config &to_this_config )
{
    simparm::NodeChoiceEntry<SpotFinderFactory>& c = 
        to_this_config.spotFindingMethod;
    c.addChoice( new Spalttiefpass::Factory() );
    c.addChoice( new MedianSmoother::Factory() );
    c.addChoice( new ErosionSmoother::Factory() );
    c.addChoice( new GaussSmoother::Factory() );
}

}
