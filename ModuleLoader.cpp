#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include "ModuleLoader.h"

#include "inputs/inputs.h"
#include "spotFinders/spotFinders.h"
#include <simparm/ChoiceEntry_Impl.hh>
#include "outputs/BasicTransmissions.h"
#include <dStorm/Config.h>
#include "engine/ChainLink_decl.h"
#include "engine_stm/ChainLink.h"
#include "noop_engine/ChainLink_decl.h"
#include "guf/fitter.h"
#include "estimate_psf_form/decl.h"

#include "test-plugin/plugin.h"
#include "locprec/plugin.h"
#include "input_simulation/plugin.h"
#include "AndorCamera/plugin.h"
#include "viewer/plugin.h"
#include "tiff/augment_config.h"
#include "andor-sif/augment_config.h"
#include "calibrate_3d/fwd.h"

#include "debug.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

namespace dStorm {

void add_modules( dStorm::Config& car_config )
{
    DEBUG("Adding basic input modules");
    dStorm::basic_inputs( &car_config );
    DEBUG("Adding rapidSTORM engine");
    car_config.add_input( engine::make_rapidSTORM_engine_link(), AsEngine );
    car_config.add_input( engine_stm::make_STM_engine_link(), AsEngine );
    car_config.add_input( noop_engine::makeLink(), AsEngine );
    DEBUG("Adding basic spot finders");
    car_config.add_spot_finder( spotFinders::make_Spalttiefpass() );
    car_config.add_spot_finder( spotFinders::make_Median() );
    car_config.add_spot_finder( spotFinders::make_Erosion() );
    car_config.add_spot_finder( spotFinders::make_Gaussian() );
    DEBUG("Adding basic output modules");
    dStorm::viewer::augment_config( car_config );
    dStorm::output::basic_outputs( &car_config );
    dStorm::tiff::augment_config( car_config );
    dStorm::andor_sif::augment_config( car_config );

    guf::augment_config( car_config );
    car_config.add_output( estimate_psf_form::make_output_source() );
    car_config.add_output( calibrate_3d::make_output_source() );
    car_config.add_output( calibrate_3d::sigma_curve::make_output_source() );
    AndorCamera::augment_config( car_config );
    locprec::augment_config( car_config );
    input_simulation::input_simulation( car_config );
    test::make_config( &car_config );

}

std::string makeProgramDescription() {
    std::stringstream ss;
    ss << PACKAGE_STRING;
    return ss.str();
}

}
