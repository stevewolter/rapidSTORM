#include "TransmissionSourceFactory.h"
#include <dStorm/BasicOutputs.h>
#include "RegionSegmenter.h"
#include "PrecisionEstimator.h"
#include "NoiseMeter.h"
#include "SpotMeter.h"
#include "SpotFinderEstimator.h"
#include "EmissionTracker.h"
#include "DensityProfile.h"

namespace locprec {

using namespace dStorm;

Outputs::Outputs()
{
    addChoice( new EmissionTracker::Source() );
    addChoice( new Segmenter::Source() );
    addChoice( new SinglePrecisionEstimator::Source() );
    addChoice( new MultiPrecisionEstimator::Source() );
    addChoice( new NoiseMeter::Source()/*, Expert )*/ );
    addChoice( new SpotMeter::Source()/*, Expert )*/ );
    addChoice( new SpotFinderEstimator::Source() );
    addChoice( new DensityProfile::Source() );
}

}
