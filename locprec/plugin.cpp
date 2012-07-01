#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "RegionSegmenter.h"
#include <dStorm/Config.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include "SpotMeter.h"
#include "PrecisionEstimator.h"
#include "VarianceEstimator.h"

using namespace dStorm::output;

namespace locprec {

void augment_config ( dStorm::Config& config ) {
    config.add_output( make_segmenter_source().release() );
    config.add_output( make_spot_meter_source().release() );
    config.add_output( make_precision_estimator_source().release() );
    config.add_output( variance_estimator::make_output_source().release() );
}

}
