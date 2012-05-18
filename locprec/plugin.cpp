#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "RegionSegmenter.h"
#include <dStorm/Config.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include "SpotMeter.h"
#include "EmissionTracker.h"
#include "DensityProfile.h"
#include "PrecisionEstimator.h"
#include "RegionOfInterest.h"
#include "SourceValuePrinter.h"
#include "RipleyK.h"
#include "VarianceEstimator.h"

using namespace dStorm::output;

namespace locprec {

void augment_config ( dStorm::Config& config ) {
    config.add_output( locprec::emission_tracker::create() );
    config.add_output( make_segmenter_source().release() );
    config.add_output( new locprec::SpotMeter::Source() )/*, Expert )*/;
    config.add_output( new locprec::DensityProfile::Source() );
    config.add_output( new locprec::PrecisionEstimator::Source() );
    config.add_output( make_roi_filter_source().release() );
    config.add_output( new locprec::SourceValuePrinter::Source() );
    config.add_output( make_output_source<ripley_k::Output>().release() );
    config.add_output( make_output_source<variance_estimator::Output>().release() );
}

}
