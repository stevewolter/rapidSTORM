#include "config.h"

#include <dStorm/engine/Image_decl.h>
#include <dStorm/input/Link.h>
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
#include "EngineChoice.h"
#include "InsertionPoint.h"
#include "InputBase.h"
#include "InputMethods.h"
#include "FileMethod.h"
#include "join.h"
#include "LocalizationFile.h"
#include "engine_stm/ChainLink.h"

namespace dStorm {

using engine::StormPixel;

void basic_inputs( dStorm::Config* config ) {
    config->add_input( make_engine_choice(), AsEngine );
    config->add_input( make_input_base(), BeforeEngine );
    config->add_input( make_insertion_place_link(AfterChannels), AfterChannels );
    config->add_input( input::join::create_link(), AfterChannels );
    config->add_input( make_insertion_place_link(BeforeChannels), BeforeChannels );
    config->add_input( inputs::InputMethods::create(), BeforeChannels );
    config->add_input( input::file_method::makeLink(), InputMethod );

    std::auto_ptr< input::Link > p = engine_stm::make_localization_buncher();
    p->insert_new_node( inputs::LocalizationFile::create(), Anywhere );
    config->add_input( p, dStorm::FileReader );

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
