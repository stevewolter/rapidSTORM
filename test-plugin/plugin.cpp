#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/ModuleInterface.h>
#include <dStorm/Config.h>
#include "SegmentationFault.h"
#include "Exception.h"
#include "MuchMemoryAllocator.h"
#include "Verbose.h"
#include "Delayer.h"
#include "BasenamePrinter.h"
#include "Manager.h"
#include "DummyFileInput.h"
#include "DummyFitter.h"
#include "RepeatTrigger.h"
#include "VerboseInputFilter_decl.h"
#include "FixedPositionSpotFinder.h"
#include <dStorm/traits/range_impl.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include "SmoothedImageSave.h"

using namespace dStorm::output;

namespace dStorm {
namespace test {

void make_config ( dStorm::Config* config ) {
    if ( !getenv("RAPIDSTORM_TESTPLUGIN_ENABLE") ) return;

    config->add_input( new dummy_file_input::Method(), dStorm::FileReader );
    config->add_input( make_verbose_input_filter(), dStorm::BeforeEngine );
    config->add_spot_finder( std::auto_ptr<dStorm::engine::spot_finder::Factory>(
        new FixedPositionSpotFinder::Finder::Factory() ) );
    config->add_spot_fitter( std::auto_ptr<dStorm::engine::spot_fitter::Factory>(
        new dStorm::debugplugin::DummyFitter::Source() ) );

    config->add_output( new SegmentationFault::Source() );
    config->add_output( new Exception::Source() );
    config->add_output( new Memalloc::Source() );
    config->add_output( new Verbose::Source() );
    config->add_output( new Delayer::Source() );
    config->add_output( new BasenamePrinter::Source() );
    config->add_output( new Repeat::Source() );
    config->add_output( new SmoothedImageSave::Source() );
}

dStorm::Display::Manager*
make_display (dStorm::Display::Manager *old)
{
    if ( getenv("DEBUGPLUGIN_LEAVE_DISPLAY") || !getenv("RAPIDSTORM_TESTPLUGIN_ENABLE") ) {
        return old;
    } else {
        std::cerr << "Test plugin loaded" << std::endl;
        return new Manager(old);
    }
}

}
}
