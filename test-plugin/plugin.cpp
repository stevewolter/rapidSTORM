#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/Config.h>
#include "Exception.h"
#include "Verbose.h"
#include "Delayer.h"
#include "BasenamePrinter.h"
#include "DummyFileInput.h"
#include "RepeatTrigger.h"
#include "VerboseInputFilter_decl.h"
#include <dStorm/traits/range_impl.h>
#include <simparm/ChoiceEntry_Impl.h>
#include "SmoothedImageSave.h"

using namespace dStorm::output;

namespace dStorm {
namespace test {

void make_config ( dStorm::Config* config ) {
    config->add_input( dummy_file_input::make(), dStorm::FileReader );
    config->add_input( make_verbose_input_filter(), dStorm::BeforeEngine );

    config->add_output( new dStorm::output::OutputBuilder< Exception::Config, Exception >() );
    config->add_output( new dStorm::output::OutputBuilder< Verbose::Config, Verbose >() );
    config->add_output( new dStorm::output::OutputBuilder< Delayer::Config, Delayer >() );
    config->add_output( make_basename_printer_source().release() );
    config->add_output( new dStorm::output::OutputBuilder< Repeat::Config, Repeat >() );
    config->add_output( new dStorm::output::OutputBuilder< SmoothedImageSave::Config, SmoothedImageSave >() );
}

}
}
