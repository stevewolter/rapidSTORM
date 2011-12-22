#include "config.h"

#include <dStorm/engine/Image_decl.h>
#include <dStorm/Config.h>
#include "TIFF.h"
#include "BackgroundDeviationEstimator.h"
#include "Splitter.h"
#include "YMirror.h"
#include "SampleInfo.h"
#include "ResolutionSetter.h"
#include "ROIFilter.h"
#include "Buffer.h"
#include "Basename.h"

namespace dStorm {

using engine::StormPixel;

void basic_inputs( dStorm::Config* config ) {
#ifdef HAVE_TIFFIO_H
    config->add_input( new TIFF::ChainLink(), dStorm::FileReader );
#endif

    config->add_input( Splitter::makeLink(), BeforeEngine );
    config->add_input( ROIFilter::make_link(), BeforeEngine );
    config->add_input( input_buffer::makeLink(), BeforeEngine );
    config->add_input( basename_input_field::makeLink(), BeforeEngine );

    config->add_input( YMirror::makeLink(), BeforeEngine );
    config->add_input( BackgroundStddevEstimator::makeLink(), BeforeEngine );
    config->add_input( input::sample_info::makeLink(), BeforeEngine );
    config->add_input( input::resolution::makeLink(), BeforeEngine );
    
}

}
