#include "debug.h"
#include "Config.h"
#include <math.h>
#include <limits>
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitter.h>
#include <dStorm/engine/SpotFitterFactory.h>

#include <simparm/ChoiceEntry_Impl.h>
#include <simparm/Entry_Impl.h>
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
    motivation("Motivation", "Spot search eagerness", 3),
    guess_threshold("GuessAmplitudeThreshold", "Guess amplitude discarding threshold", true),
    threshold_height_factor("ThresholdHeightFactor", "Amplitude threshold proportionality", 35.0f),
    amplitude_threshold("AmplitudeThreshold", "Amplitude discarding threshold", 1000 * camera::ad_count)
{
    DEBUG("Building dStorm Config");

    nms.set_user_level(simparm::Intermediate);
    
    motivation.setHelp("Abort spot search when this many successive "
                        "bad candidates are found.");
    motivation.set_user_level(simparm::Intermediate);

    guess_threshold.set_user_level(simparm::Beginner);
    threshold_height_factor.set_user_level(simparm::Expert);
    amplitude_threshold.set_user_level(simparm::Beginner);
    amplitude_threshold.hide();
    amplitude_threshold.setHelp("Every fit attempt with an amplitude higher "
                                "than this threshold will be considered a "
                                "localization, and those below the threshold "
                                "are discarded immediately. Compared with the "
                                "other amplitude threshold in the Viewer, this "
                                "threshold is already enforced during computation,"
                                "thus saving computation time and avoiding false "
                                "positives; however, contrary to the other threshold, "
                                "it's application is not reversible.");

    amplitude_threshold.setHelpID( "#AmplitudeThreshold" );
    spotFindingMethod.setHelpID( "#Smoother" );
    spotFindingMethod.set_user_level( simparm::Intermediate );
    spotFittingMethod.set_user_level( simparm::Intermediate );

    spotFindingMethod.set_auto_selection( true );

    spotFittingMethod.set_auto_selection( true );
}

Config::~Config() {
    /* No special stuff here. Declared in this file to avoid necessity
     * of including simparm::ChoiceEntry implementation when using
     * Config. */
}

void Config::attach_ui( simparm::NodeHandle n ) {
    simparm::NodeHandle at = name_object.attach_ui( n );
    nms.attach_ui(at);
    guess_threshold.attach_ui(at);
    threshold_height_factor.attach_ui(at);
    amplitude_threshold.attach_ui(at);

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
    std::stringstream ss;
    if ( guess_threshold() ) 
        ss << "auto";
    else
        ss << amplitude_threshold().value();
    bn.set_variable("thres", ss.str());

    spotFindingMethod().set_variables( bn );
    spotFittingMethod().set_variables( bn );
}

}
}

namespace simparm {

template class Entry< dStorm::engine::Config::PixelVector2D >;

}
