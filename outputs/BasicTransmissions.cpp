#include "BasicTransmissions.h"

#include "LocalizationFile.h"
#include "Viewer.h"
#include "ViewerConfig.h"
#include "LocalizationCounter.h"
#include "ProgressMeter.h"
#include "AverageImage.h"
#include "TraceFilter.h"
#include "LocalizationFilter.h"
#include "Slicer.h"
#include "RawImageFile.h"
#include "PrecisionEstimator.h"

using namespace std;
using namespace locprec;

namespace dStorm {

void basic_outputs( dStorm::BasicOutputs* o ) {
    o->addChoice( new LocalizationFile::Source() );
    o->addChoice( new Viewer::Source() );
    o->addChoice( new ProgressMeter::Source() );
    o->addChoice( new LocalizationCounter::Source() );
    o->addChoice( new AverageImage::Source() );
    o->addChoice( new LocalizationFilter::Source() );
    o->addChoice( new TraceCountFilter::Source() );
    o->addChoice( new Slicer::Source() );
    o->addChoice( new RawImageFile::Source() );
    o->addChoice( new SinglePrecisionEstimator::Source() );
    o->addChoice( new MultiPrecisionEstimator::Source() );
}

}
