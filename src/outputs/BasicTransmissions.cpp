#define CImgBuffer_TIFFLOADER_CPP
#include "BasicTransmissions.h"

#include "LocalizationFile.h"
#include "../viewer/plugin.h"
#include "LocalizationCounter.h"
#include "ProgressMeter.h"
#include "AverageImage.h"
#include <dStorm/outputs/TraceFilter.h>
#include "LocalizationFilter.h"
#include "Slicer.h"
#include "RawImageFile.h"
#include "PrecisionEstimator.h"

using namespace std;
using namespace dStorm::outputs;

namespace dStorm {
namespace output {

void basic_outputs( Config* o ) {
    o->addChoice( new LocalizationFile::Source() );
    outputs::add_viewer( *o );
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
}
