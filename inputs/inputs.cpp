#include "config.h"

#include <dStorm/engine/Image_decl.h>
#include <dStorm/input/Config.h>
#include "TIFF.h"
#include "BackgroundDeviationEstimator_decl.h"
#include "Splitter_decl.h"
#include "YMirror_decl.h"
#include "SampleInfo.h"
#include "ResolutionSetter_decl.h"
#include "ROIFilter_decl.h"
#include "Buffer_decl.h"
#include "Basename_decl.h"

namespace dStorm {

using engine::StormPixel;

void basic_inputs( input::Config* inputConfig ) {
#ifdef HAVE_TIFFIO_H
    inputConfig->add_method( new TIFF::ChainLink() );
#endif

    inputConfig->add_filter( ROIFilter::makeFilter() );
    inputConfig->add_filter( input::makeBufferChainLink() );
    inputConfig->add_filter( input::Basename::makeLink() );

    inputConfig->add_filter( Splitter::makeLink(), true );
    inputConfig->add_filter( YMirror::makeLink() );
    inputConfig->add_filter( BackgroundStddevEstimator::makeLink() );
    inputConfig->add_filter( input::sample_info::makeLink() );
    inputConfig->add_filter( input::Resolution::makeLink() );
    
}

}
