#include "RegionSegmenterConfig.h"
#include "RegionSegmenter.h"
#include "MaximumSearcher.h"
#include "PrecisionEstimator.h"
#include "NoiseMeter.h"
#include "SpotMeter.h"
#include "SpotFinderEstimator.h"

namespace locprec {

using namespace dStorm;

std::auto_ptr<TransmissionSource> LocprecTransmissionSourceFactory
    ::makeTransmissionSource(std::string what) throw() 
{
    std::auto_ptr<TransmissionSource> basic = 
        BasicTransmissions::makeTransmissionSource(what);
    TransmissionChooser* chooser = 
        dynamic_cast<TransmissionChooser*>( basic.get() );
    if ( chooser != NULL ) {
        chooser->addChoice( new SegmenterConfig(*this) );
        chooser->addChoice( new NoiseMeterConfig() );
        chooser->addChoice( new SpotMeterConfig() );
        chooser->addChoice( new SpotFinderEstimatorConfig() );
    }

    return basic;
}

}
