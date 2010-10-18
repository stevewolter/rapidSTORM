#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
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
#include <simparm/ChoiceEntry_Impl.hh>

using namespace dStorm::output;

static Manager *manager = NULL;

OutputSource* write_help_file( OutputSource *src ) {
    src->help_file = "";
    return src;
}

#ifdef __cplusplus
extern "C" {
#endif

const char * rapidSTORM_Plugin_Desc() {
    return "Tests";
}

void rapidSTORM_Config_Augmenter ( dStorm::Config* config ) {
    config->inputConfig.add_file_method( new dummy_file_input::Method() );
    config->engineConfig.spotFittingMethod.addChoice( new dStorm::debugplugin::DummyFitter::Source() );

    dStorm::output::Config* outputs = &config->outputConfig;
    try {
        outputs->addChoice( write_help_file( new SegmentationFault::Source() ) );
        outputs->addChoice( write_help_file( new Exception::Source() ) );
        outputs->addChoice( write_help_file( new Memalloc::Source() ) );
        outputs->addChoice( write_help_file( new Verbose::Source() ) );
        outputs->addChoice( write_help_file( new Delayer::Source() ) );
        outputs->addChoice( write_help_file( new BasenamePrinter::Source() ) );
    } catch ( const std::exception& e ) {
        std::cerr << "Unable to add debug plugin: " << e.what() << "\n";
    }
}

dStorm::Display::Manager*
rapidSTORM_Display_Driver
    (dStorm::Display::Manager *old)
{
    if ( getenv("DEBUGPLUGIN_LEAVE_DISPLAY") ) {
        return old;
    } else {
        std::cerr << "Test plugin loaded" << std::endl;
        manager = new Manager(old);
        return manager;
    }
}

void
    rapidSTORM_Cleanup_Handler
    (dStorm::ErrorHandler::CleanupArgs* , 
     dStorm::JobMaster* )
{
}

#ifdef __cplusplus
}
#endif
