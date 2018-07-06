#include "engine/SpotFinder.h"
#include "engine/SpotFitterFactory.h"
#include "base/Config.h"
#include "test-plugin/Exception.h"
#include "test-plugin/Verbose.h"
#include "test-plugin/Delayer.h"
#include "test-plugin/BasenamePrinter.h"
#include "test-plugin/RepeatTrigger.h"

using namespace dStorm::output;

namespace dStorm {
namespace test {

void output_modules( dStorm::Config* config ) {
    config->add_output( new dStorm::output::OutputBuilder< Exception::Config, Exception >() );
    config->add_output( new dStorm::output::OutputBuilder< Verbose::Config, Verbose >() );
    config->add_output( new dStorm::output::OutputBuilder< Delayer::Config, Delayer >() );
    config->add_output( make_basename_printer_source().release() );
    config->add_output( new dStorm::output::OutputBuilder< Repeat::Config, Repeat >() );
}

}
}
