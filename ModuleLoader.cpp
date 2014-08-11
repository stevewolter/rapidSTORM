#include "ModuleLoader.h"

#include "andor-sif/AndorSIF.h"
#include "base/Config.h"
#include "calibrate_3d/fwd.h"
#include "engine/ChainLink.h"
#include "engine/SpotFinder.h"
#include "engine/SpotFitterFactory.h"
#include "engine_stm/ChainLink.h"
#include "estimate_psf_form/decl.h"
#include "helpers/make_unique.hpp"
#include "input_simulation/NoiseSource.h"
#include "input/Choice.h"
#include "input/FilterFactoryLink.hpp"
#include "inputs/inputs.h"
#include "inputs/MedianFilter.h"
#include "inputs/WarnAboutLocalizationFile.h"
#include "kalman_filter/EmissionTracker.h"
#include "kalman_filter/NonlinearDriftEstimator.h"
#include "localization_file/writer.h"
#include "outputs/BasicTransmissions.h"
#include "ripley_k/fwd.h"
#include "test-plugin/DummyFileInput.h"
#include "test-plugin/plugin.h"
#include "tiff/RawImageFile.h"
#include "tiff/TIFF.h"
#include "viewer/plugin.h"

#include "debug.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_PROTOBUF
#include "tsf/Output.h"
#endif

namespace dStorm {

std::unique_ptr<input::Link<output::LocalizedImage>> create_image_input() {
    auto file_methods = make_unique<inputs::FileMethod<engine::ImageStack>>();
#ifdef HAVE_TIFFIO_H
    file_methods->add_choice(tiff::make_input());
#endif
#ifdef HAVE_LIBREADSIF
    file_methods->add_choice(andor_sif::make_input());
#endif
    file_methods->add_choice(inputs::make_warn_about_localization_file());
    file_methods->add_choice(dummy_file_input::make());

    auto input_methods = make_unique<input::Choice<engine::ImageStack>>(
        "InputMethod", false);
    input_methods->add_choice(std::move(file_methods));
    input_methods->add_choice(make_unique<input_simulation::NoiseConfig>());

    std::unique_ptr<input::Link<engine::ImageStack>> image_link =
        std::move(input_methods);
    image_link = input::CreateLink<engine::ImageStack, engine::ImageStack>(
        ROIFilter::create(), std::move(image_link));
    image_link = CreateLink(YMirror::create(), std::move(image_link));
    image_link = inputs::join::create_image_link(std::move(image_link));

    image_link = CreateLink(splitter::create(), std::move(image_link));
    image_link = CreateLink(plane_filter::create(), std::move(image_link));
    image_link = CreateLink(median_filter::create(), std::move(image_link));
    image_link = CreateLink(input_buffer::create(), std::move(image_link));
    image_link = basename_input_field::makeImageLink(std::move(image_link));
    image_link = CreateLink(input::sample_info::create(), std::move(image_link));
    image_link = CreateLink(input::resolution::create(), std::move(image_link));

    image_link = make_image_input_base(std::move(image_link));
    return CreateLink(engine::make_rapidSTORM_engine_link(), std::move(image_link));
}

std::unique_ptr<input::Link<output::LocalizedImage>> create_localizations_input() {
    auto file_methods = make_unique<inputs::FileMethod<output::LocalizedImage>>();
    file_methods->add_choice(
        CreateLink(engine_stm::create(),inputs::LocalizationFile::create()));

    auto input_methods = make_unique<input::Choice<output::LocalizedImage>>(
        "InputMethod", false);
    input_methods->add_choice(std::move(file_methods));

    return make_localization_input_base(
        basename_input_field::makeLocalizationLink(
            inputs::join::create_localization_link(
                std::move(input_methods))));
}

void add_output_modules( dStorm::Config& car_config ) {
    dStorm::viewer::augment_config( car_config );
    dStorm::output::basic_outputs( &car_config );

#ifdef HAVE_TIFFIO_H
    car_config.add_output( new output::FileOutputBuilder<output::RawImageFile::Config,output::RawImageFile>() );
#endif

    car_config.add_output( estimate_psf_form::make_output_source() );
    car_config.add_output( calibrate_3d::make_output_source() );
    car_config.add_output( calibrate_3d::sigma_curve::make_output_source() );
    car_config.add_output( kalman_filter::emission_tracker::create() );
    car_config.add_output( kalman_filter::drift_estimator::create() );
    car_config.add_output( localization_file::make_output_source() );
    car_config.add_output( ripley_k::make_output_source().release() );
    test::output_modules( &car_config );

#if HAVE_PROTOBUF
    car_config.add_output( tsf::CreateOutput() );
#endif
}

}
