#define CImgBuffer_TIFFLOADER_CPP
#include "outputs/BasicTransmissions.h"

#include "outputs/LocalizationCounter.h"
#include "outputs/ProgressMeter.h"
#include "outputs/AverageImage.h"
#include "outputs/TraceFilter.h"
#include "expression/Source_decl.h"
#include "outputs/Slicer.h"
#include "outputs/MemoryCache.h"
#include "outputs/LocalizationFile.h"
#include "outputs/SigmaDiff3D.h"
#include "outputs/LinearAlignment.h"
#include "outputs/DriftRemover.h"
#include "outputs/RegionOfInterest.h"
#include "outputs/RegionSegmenter.h"
#include "outputs/SpotMeter.h"
#include "outputs/PrecisionEstimator.h"
#include "outputs/VarianceEstimator.h"

using namespace std;
using namespace dStorm::outputs;

namespace dStorm {
namespace output {

void basic_outputs( dStorm::Config* o ) {
    o->add_output( localization_file::writer::create() );
    o->add_output( make_progress_meter_source().release() );
    o->add_output( make_localization_counter_source().release() );
    o->add_output( make_average_image_source().release() );
    o->add_output( memory_cache::make_output_source().release() );
    o->add_output( make_trace_count_source().release() );
    o->add_output( slicer::make_output_source() );
    o->add_output( expression::make_output_source().release() );
    o->add_output( make_sigma_diff_3d().release() );
    o->add_output( make_linear_alignment().release() );
    o->add_output( drift_remover::make().release() );
    o->add_output( make_roi_filter_source().release() );
    o->add_output( make_segmenter_source().release() );
    o->add_output( make_spot_meter_source().release() );
    o->add_output( make_precision_estimator_source().release() );
    o->add_output( make_variance_estimator_source().release() );
}

}
}
