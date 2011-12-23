#define CImgBuffer_TIFFLOADER_CPP
#include "BasicTransmissions.h"

#include "../viewer/plugin.h"
#include "LocalizationCounter.h"
#include "ProgressMeter.h"
#include "AverageImage.h"
#include <dStorm/outputs/TraceFilter.h>
#include <dStorm/expression/Source_decl.h>
#include "Slicer.h"
#include "RawImageFile.h"
#include "MemoryCache.h"
#include "LocalizationFile.h"

using namespace std;
using namespace dStorm::outputs;

namespace dStorm {
namespace output {

void basic_outputs( dStorm::Config* o ) {
    o->add_output( localization_file::writer::create() );
    outputs::add_viewer( *o );
    o->add_output( new ProgressMeter::Source() );
    o->add_output( new LocalizationCounter::Source() );
    o->add_output( new AverageImage::Source() );
    o->add_output( memory_cache::make_output_source().release() );
    o->add_output( new TraceCountFilter::Source() );
    o->add_output( new Slicer::Source() );
    o->add_output( new RawImageFile::Source() );
    o->add_output( make_output_source<expression::Source>().release() );
}

}
}
