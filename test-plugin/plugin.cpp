#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/ModuleInterface.h>
#include <dStorm/Config.h>
#include "Exception.h"
#include "Verbose.h"
#include "Delayer.h"
#include "BasenamePrinter.h"
#include "Manager.h"
#include "DummyFileInput.h"
#include "RepeatTrigger.h"
#include "VerboseInputFilter_decl.h"
#include <dStorm/traits/range_impl.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include "SmoothedImageSave.h"

using namespace dStorm::output;

namespace dStorm {
namespace test {

void make_config ( dStorm::Config* config ) {
    if ( !getenv("RAPIDSTORM_TESTPLUGIN_ENABLE") ) return;

    config->add_input( dummy_file_input::make(), dStorm::FileReader );
    config->add_input( make_verbose_input_filter(), dStorm::BeforeEngine );

    config->add_output( new dStorm::output::OutputBuilder< Exception::Config, Exception >() );
    config->add_output( new dStorm::output::OutputBuilder< Verbose::Config, Verbose >() );
    config->add_output( new dStorm::output::OutputBuilder< Delayer::Config, Delayer >() );
    config->add_output( make_basename_printer_source().release() );
    config->add_output( new dStorm::output::OutputBuilder< Repeat::Config, Repeat >() );
    config->add_output( new dStorm::output::OutputBuilder< SmoothedImageSave::Config, SmoothedImageSave >() );
}

dStorm::display::Manager*
make_display (dStorm::display::Manager *old)
{
    if ( getenv("DEBUGPLUGIN_LEAVE_DISPLAY") || !getenv("RAPIDSTORM_TESTPLUGIN_ENABLE") ) {
        return old;
    } else {
        std::cerr << "Test plugin loaded" << std::endl;
        return make_test_plugin_manager( old ).release();
    }
}

}
}
