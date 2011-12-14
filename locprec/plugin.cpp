#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "NoiseSource.h"
#include <dStorm/ModuleInterface.h>
#include <dStorm/Config.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include "RegionSegmenter.h"
#include "NoiseMeter.h"
#include "SpotMeter.h"
//#include "locprec/SpotFinderEstimator.h"
#include "EmissionTracker_decl.h"
#include "DensityProfile.h"
#include "FillholeSmoother.h"
#include "PrecisionEstimator.h"
#include "RegionOfInterest.h"
#include "SourceValuePrinter.h"
#include "biplane_alignment/decl.h"
#include "RipleyK.h"
#include "VarianceEstimator.h"

using namespace dStorm::output;

#ifdef __cplusplus
extern "C" {
#endif

const char * rapidSTORM_Plugin_Desc() {
    return PACKAGE_STRING;
}

void rapidSTORM_Config_Augmenter ( dStorm::Config* config ) {
    config->inputConfig.add_method( new locprec::NoiseConfig(), dStorm::input::chain::Link::InputMethod );
    //config->inputConfig.add_filter( locprec::biplane_alignment::make_filter() );
    config->add_spot_finder( 
        new locprec::FillholeSmoother::Factory() );
    config->outputConfig.addChoice( make_output_source<locprec::emission_tracker::Output>().release() );
    config->outputConfig.addChoice( new locprec::Segmenter::Source() );
    config->outputConfig.addChoice( new locprec::NoiseMeter::Source() ) /*, Expert )*/;
    config->outputConfig.addChoice( new locprec::SpotMeter::Source() )/*, Expert )*/;
    //config->outputConfig.addChoice( write_help_file( new locprec::SpotFinderEstimator::Source() ) );
    config->outputConfig.addChoice( new locprec::DensityProfile::Source() );
    config->outputConfig.addChoice( new locprec::PrecisionEstimator::Source() );
    config->outputConfig.addChoice( new locprec::ROIFilter::Source() );
    config->outputConfig.addChoice( new locprec::SourceValuePrinter::Source() );
    config->outputConfig.addChoice( make_output_source<ripley_k::Output>().release() );
    config->outputConfig.addChoice( make_output_source<variance_estimator::Output>().release() );
}

dStorm::Display::Manager*
rapidSTORM_Display_Driver
    (dStorm::Display::Manager *current)
{
    return current;
}

#ifdef __cplusplus
}
#endif
