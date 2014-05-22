#include "debug.h"
#include "engine/Config.h"
#include <math.h>
#include <limits>
#include "engine/SpotFinder.h"
#include "engine/SpotFitter.h"
#include "engine/SpotFitterFactory.h"

#include "output/Basename.h"
#include <boost/bind/bind.hpp>

#include "config.h"

namespace dStorm {
namespace engine {

Config::Config()
:   name_object("rapidSTORM", "rapidSTORM engine"),
    fit_position_epsilon("FitPositionEpsilon", 250 * si::nanometre),
    separate_plane_fitting("SeparatePlaneFitting", false),
    spotFindingMethod("SpotFindingMethod"),
    spotFittingMethod("SpotFittingMethod"),
    fit_judging_method("FitJudgingMethod"),
    motivation("Motivation", 3)
{
    DEBUG("Building dStorm Config");

    motivation.setHelp("Abort spot search when this many successive "
                        "bad candidates are found.");
    motivation.set_user_level(simparm::Intermediate);

    separate_plane_fitting.set_user_level(simparm::Expert);

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
    fit_position_epsilon.attach_ui(at);
    fit_judging_method.attach_ui(at);

    spotFindingMethod.attach_ui(at);
    separate_plane_fitting.attach_ui(at);
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
