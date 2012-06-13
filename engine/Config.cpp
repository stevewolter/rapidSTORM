#include "debug.h"
#include "Config.h"
#include <math.h>
#include <limits>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitter.h>
#include <dStorm/engine/SpotFitterFactory.h>

#include <dStorm/output/Basename.h>
#include <boost/bind/bind.hpp>

#include "config.h"

namespace dStorm {
namespace engine {

Config::Config()
:   name_object("rapidSTORM", "rapidSTORM engine"),
    nms("NonMaximumSuppression", "Minimum spot distance", PixelVector2D::Constant(3 * camera::pixel) ),
    spotFindingMethod("SpotFindingMethod", "Spot finding method"),
    weights("SpotFindingWeights", "Spot finding weights"),
    spotFittingMethod("SpotFittingMethod", "Spot fitting method"),
    fit_judging_method("FitJudgingMethod", "Fit judging method"),
    motivation("Motivation", "Spot search eagerness", 3)
{
    DEBUG("Building dStorm Config");

    nms.set_user_level(simparm::Intermediate);
    
    motivation.setHelp("Abort spot search when this many successive "
                        "bad candidates are found.");
    motivation.set_user_level(simparm::Intermediate);

    spotFindingMethod.setHelpID( "#Smoother" );
    spotFindingMethod.set_user_level( simparm::Intermediate );
    spotFittingMethod.set_user_level( simparm::Beginner );

    fit_judging_method.addChoice( make_fixed_threshold_judger() );
    fit_judging_method.addChoice( make_square_root_ratio_judger() );

    spotFindingMethod.set_auto_selection( true );
    spotFittingMethod.set_auto_selection( true );
    fit_judging_method.set_auto_selection( true );
}

Config::~Config() {
    /* No special stuff here. Declared in this file to avoid necessity
     * of including simparm::ChoiceEntry implementation when using
     * Config. */
}

void Config::attach_ui( simparm::NodeHandle n ) {
    simparm::NodeHandle at = name_object.attach_ui( n );
    nms.attach_ui(at);
    fit_judging_method.attach_ui(at);

    spotFindingMethod.attach_ui(at);
    simparm::NodeHandle w = weights.attach_ui(at );
    weights_insertion_point = w;
    std::for_each( spot_finder_weights.begin(), spot_finder_weights.end(),
        boost::bind( &simparm::Entry<float>::attach_ui, _1, w ) );
    spotFittingMethod.attach_ui(at);

    motivation.attach_ui(at);

}


void Config::set_variables( output::Basename& bn ) const
{
    fit_judging_method().set_variables( bn );
    spotFindingMethod().set_variables( bn );
    spotFittingMethod().set_variables( bn );
}

}
}

namespace simparm {

template class Entry< dStorm::engine::Config::PixelVector2D >;

}
