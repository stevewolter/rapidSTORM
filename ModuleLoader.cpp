#include "engine/SpotFinder.h"
#include "engine/SpotFitterFactory.h"
#include "ModuleLoader.h"

#include "inputs/inputs.h"
#include "inputs/WarnAboutLocalizationFile.h"
#include "spotFinders/spotFinders.h"
#include "outputs/BasicTransmissions.h"
#include "base/Config.h"
#include "engine/ChainLink_decl.h"
#include "engine_stm/ChainLink.h"
#include "noop_engine/ChainLink_decl.h"
#include "guf/fitter.h"
#include "estimate_psf_form/decl.h"
#include "localization_file/writer.h"

#include "test-plugin/plugin.h"
#include "kalman_filter/fwd.h"
#include "input_simulation/plugin.h"
#include "viewer/plugin.h"
#include "tiff/augment_config.h"
#include "andor-sif/augment_config.h"
#include "calibrate_3d/fwd.h"
#include "ripley_k/fwd.h"

#include "debug.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_PROTOBUF
#include "tsf/Output.h"
#endif

namespace dStorm {

void add_image_input_modules( dStorm::Config& car_config )
{
    car_config.add_input( engine::make_rapidSTORM_engine_link(), AsEngine );

    car_config.add_input( make_input_base(), BeforeEngine );
    car_config.add_input( make_insertion_place_link(AfterChannels), AfterChannels );
    car_config.add_input( input::join::create_link(), AfterChannels );
    car_config.add_input( make_insertion_place_link(BeforeChannels), BeforeChannels );
    car_config.add_input( inputs::InputMethods::create(), BeforeChannels );
    car_config.add_input( input::file_method::makeLink(), InputMethod );

    car_config.add_input( ROIFilter::make_link(), BeforeChannels );
    car_config.add_input( YMirror::makeLink(), BeforeChannels );

    car_config.add_input( Splitter::makeLink(), BeforeEngine );
    car_config.add_input( plane_filter::make_link(), BeforeEngine );
    car_config.add_input( input_buffer::makeLink(), BeforeEngine );
    car_config.add_input( basename_input_field::makeLink(), BeforeEngine );

    car_config.add_input( input::sample_info::makeLink(), BeforeEngine );
    car_config.add_input( input::resolution::makeLink(), BeforeEngine );
    input_simulation::input_simulation( car_config );
    dStorm::tiff::input_driver( car_config );
    dStorm::andor_sif::augment_config( car_config );
    car_config.add_input( inputs::make_warn_about_localization_file(), FileReader );

    car_config.add_spot_finder( spalttiefpass_smoother::make_spot_finder_factory() );
    car_config.add_spot_finder( median_smoother::make_spot_finder_factory() );
    car_config.add_spot_finder( erosion_smoother::make_spot_finder_factory() );
    car_config.add_spot_finder( gauss_smoother::make_spot_finder_factory() );
    car_config.add_spot_finder( spaltbandpass_smoother::make_spot_finder_factory() );
    guf::augment_config( car_config );
    test::input_modules( &car_config );
}

void add_stm_input_modules( dStorm::Config& car_config )
{
    car_config.add_input( engine_stm::make_STM_engine_link(), AsEngine );
    car_config.add_input( make_input_base(), BeforeEngine );
    car_config.add_input( input::join::create_link(), AfterChannels );
    car_config.add_input( inputs::InputMethods::create(), BeforeChannels );
    car_config.add_input( input::file_method::makeLink(), InputMethod );

    std::auto_ptr< input::Link > p = engine_stm::make_localization_buncher();
    p->insert_new_node( inputs::LocalizationFile::create(), Anywhere );
    car_config.add_input( p, dStorm::FileReader );

    car_config.add_input( basename_input_field::makeLink(), BeforeEngine );
}

void add_output_modules( dStorm::Config& car_config )
{
    dStorm::viewer::augment_config( car_config );
    dStorm::output::basic_outputs( &car_config );
    dStorm::tiff::output_driver( car_config );

    car_config.add_output( estimate_psf_form::make_output_source() );
    car_config.add_output( calibrate_3d::make_output_source() );
    car_config.add_output( calibrate_3d::sigma_curve::make_output_source() );
    car_config.add_output( kalman_filter::create() );
    car_config.add_output( kalman_filter::create_drift_correction() );
    car_config.add_output( localization_file::make_output_source() );
    car_config.add_output( ripley_k::make_output_source().release() );
    test::output_modules( &car_config );

#if HAVE_PROTOBUF
    car_config.add_output( tsf::CreateOutput() );
#endif
}

}
