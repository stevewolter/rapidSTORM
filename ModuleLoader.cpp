#include "engine/SpotFinder.h"
#include "engine/SpotFitterFactory.h"
#include "ModuleLoader.h"

#include "input/Choice.h"
#include "inputs/inputs.h"
#include "inputs/WarnAboutLocalizationFile.h"
#include "outputs/BasicTransmissions.h"
#include "base/Config.h"
#include "engine/ChainLink_decl.h"
#include "engine_stm/ChainLink.h"
#include "noop_engine/ChainLink_decl.h"
#include "estimate_psf_form/decl.h"
#include "localization_file/writer.h"

#include "test-plugin/plugin.h"
#include "test-plugin/DummyFileInput.h"
#include "kalman_filter/fwd.h"
#include "input_simulation/NoiseSource.h"
#include "viewer/plugin.h"
#include "tiff/RawImageFile.h"
#include "tiff/TIFF.h"
#include "calibrate_3d/fwd.h"
#include "ripley_k/fwd.h"
#include "inputs/MedianFilter.h"

#include "debug.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_PROTOBUF
#include "tsf/Output.h"
#endif

#if HAVE_LIBREADSIF
#include "andor-sif/AndorSIF.h"
#endif

namespace dStorm {

void add_image_input_modules( dStorm::Config& car_config )
{
    car_config.add_input( engine::make_rapidSTORM_engine_link(), AsEngine );

    inputs::FileMethod* file_method = new inputs::FileMethod();
#ifdef HAVE_TIFFIO_H
    file_method->add_choice("TIFF", tiff::make_input());
#endif
#ifdef HAVE_LIBREADSIF
    file_method->add_choice("AndorSIF", andor_sif::make_input());
#endif
    file_method->add_choice("STM", inputs::make_warn_about_localization_file());
    file_method->add_choice("DummyInput", dummy_file_input::make());

    input::Choice* input_methods = new input::Choice("InputMethod", true);
    input_methods->add_choice("FileMethod", std::auto_ptr<input::Link>(file_method));
    input_methods->add_choice("Generated", std::auto_ptr<input::Link>(new input_simulation::NoiseConfig()));

    car_config.add_input( make_input_base(), BeforeEngine );
    car_config.add_input( make_insertion_place_link(AfterChannels), AfterChannels );
    car_config.add_input( inputs::join::create_link(), AfterChannels );
    car_config.add_input( make_insertion_place_link(BeforeChannels), BeforeChannels );
    car_config.add_input( std::auto_ptr<input::Link>(input_methods), BeforeChannels );

    car_config.add_input( ROIFilter::make_link(), BeforeChannels );
    car_config.add_input( YMirror::makeLink(), BeforeChannels );

    car_config.add_input( Splitter::makeLink(), BeforeEngine );
    car_config.add_input( plane_filter::make_link(), BeforeEngine );
    car_config.add_input( median_filter::make_link(), BeforeEngine );
    car_config.add_input( input_buffer::makeLink(), BeforeEngine );
    car_config.add_input( basename_input_field::makeLink(), BeforeEngine );

    car_config.add_input( input::sample_info::makeLink(), BeforeEngine );
    car_config.add_input( input::resolution::makeLink(), BeforeEngine );
}

void add_stm_input_modules( dStorm::Config& car_config )
{
    car_config.add_input( engine_stm::make_STM_engine_link(), AsEngine );

    std::auto_ptr< input::Link > p = engine_stm::make_localization_buncher();
    p->insert_new_node( inputs::LocalizationFile::create(), Anywhere );

    inputs::FileMethod* file_method = new inputs::FileMethod();
    file_method->add_choice("STM", p);

    input::Choice* input_methods = new input::Choice("InputMethod", false);
    input_methods->add_choice("FileMethod",
                              std::auto_ptr<input::Link>(file_method));

    car_config.add_input( make_input_base(), BeforeEngine );
    car_config.add_input( inputs::join::create_link(), AfterChannels );

    car_config.add_input( basename_input_field::makeLink(), BeforeEngine );
}

void add_output_modules( dStorm::Config& car_config )
{
    dStorm::viewer::augment_config( car_config );
    dStorm::output::basic_outputs( &car_config );

#ifdef HAVE_TIFFIO_H
    car_config.add_output( new output::FileOutputBuilder<output::RawImageFile::Config,output::RawImageFile>() );
#endif

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
