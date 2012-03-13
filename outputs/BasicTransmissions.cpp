#define CImgBuffer_TIFFLOADER_CPP
#include "BasicTransmissions.h"

#include "LocalizationCounter.h"
#include "ProgressMeter.h"
#include "AverageImage.h"
#include <dStorm/outputs/TraceFilter.h>
#include "expression/Source_decl.h"
#include "Slicer.h"
#include "MemoryCache.h"
#include "LocalizationFile.h"
#include "calibrate_3d/fwd.h"

using namespace std;
using namespace dStorm::outputs;

namespace dStorm {
namespace output {

void basic_outputs( dStorm::Config* o ) {
    o->add_output( localization_file::writer::create() );
    o->add_output( new ProgressMeter::Source() );
    o->add_output( new LocalizationCounter::Source() );
    o->add_output( new AverageImage::Source() );
    o->add_output( memory_cache::make_output_source().release() );
    o->add_output( new TraceCountFilter::Source() );
    o->add_output( new Slicer::Source() );
    o->add_output( make_output_source<expression::Source>().release() );
    o->add_output( calibrate_3d::make_output_source() );
}

}
}
