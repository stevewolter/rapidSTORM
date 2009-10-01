#include "OutputSource.h"
#include "FilterSource.h"
#include <list>

#include "transmissions/LocalizationFile.h"
#include "transmissions/Viewer.h"
#include "transmissions/ViewerConfig.h"
#include "transmissions/LocalizationCounter.h"
#include "transmissions/ProgressMeter.h"
#include "transmissions/AverageImage.h"
#include "transmissions/TraceFilter.h"
#include "transmissions/LocalizationFilter.h"
#include "transmissions/Slicer.h"
#include "transmissions/RawImageFile.h"
#include <simparm/Object.hh>
#include "help_context.h"

#include "BasicOutputs.h"

using namespace std;

namespace dStorm {

#if 0
    addChoice( new LocalizationFile::Source() );
    addChoice( new Viewer::Source() );
    addChoice( new ProgressMeter::Source() );
    addChoice( new LocalizationCounter::Source() );
    addChoice( new AverageImage::Source() );
    addChoice( new LocalizationFilter::Source() );
    addChoice( new TraceCountFilter::Source() );
    addChoice( new Slicer::Source() );
    addChoice( new RawImageFile::Source() );
#endif

}
