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

static Manager *manager = NULL;

#ifdef __cplusplus
extern "C" {
#endif

const char * rapidSTORM_Plugin_Desc() {
    if ( getenv("RAPIDSTORM_TESTPLUGIN_ENABLE") )
        return "Tests";
    else
        return NULL;
}

void rapidSTORM_Config_Augmenter ( dStorm::Config* config ) {
    if ( !getenv("RAPIDSTORM_TESTPLUGIN_ENABLE") ) return;

    config->inputConfig.add_method( new dummy_file_input::Method() );
    config->inputConfig.add_filter( make_verbose_input_filter() );
    config->add_spot_finder( std::auto_ptr<dStorm::engine::spot_finder::Factory>(
        new FixedPositionSpotFinder::Finder::Factory() ) );
    config->add_spot_fitter( std::auto_ptr<dStorm::engine::spot_fitter::Factory>(
        new dStorm::debugplugin::DummyFitter::Source() ) );

    dStorm::output::Config* outputs = &config->outputConfig;
    try {
        outputs->addChoice( new SegmentationFault::Source() );
        outputs->addChoice( new Exception::Source() );
        outputs->addChoice( new Memalloc::Source() );
        outputs->addChoice( new Verbose::Source() );
        outputs->addChoice( new Delayer::Source() );
        outputs->addChoice( new BasenamePrinter::Source() );
        outputs->addChoice( new Repeat::Source() );
        outputs->addChoice( new SmoothedImageSave::Source() );
    } catch ( const std::exception& e ) {
        std::cerr << "Unable to add debug plugin: " << e.what() << "\n";
    }
}

dStorm::Display::Manager*
rapidSTORM_Display_Driver
    (dStorm::Display::Manager *old)
{
    if ( getenv("DEBUGPLUGIN_LEAVE_DISPLAY") || !getenv("RAPIDSTORM_TESTPLUGIN_ENABLE") ) {
        return old;
    } else {
        std::cerr << "Test plugin loaded" << std::endl;
        manager = new Manager(old);
        return manager;
    }
}

#ifdef __cplusplus
}
#endif
