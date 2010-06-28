#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dStorm/ModuleInterface.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include "Factory.h"

using namespace dStorm::gauss_3d_fitter;

#ifdef __cplusplus
extern "C" {
#endif

const char * rapidSTORM_Plugin_Desc() {
    return "Cylens fitter";
}

void rapidSTORM_Input_Augmenter ( dStorm::input::Config* inputs ) {
}

void rapidSTORM_Engine_Augmenter
    ( dStorm::engine::Config* config )
{
    config->spotFittingMethod.addChoice( new Factory() );
}

void rapidSTORM_Output_Augmenter
    ( dStorm::output::Config* outputs )
{
}

dStorm::Display::Manager*
rapidSTORM_Display_Driver
    (dStorm::Display::Manager *old)
{
    return old;
}

void
    rapidSTORM_Cleanup_Handler
    (dStorm::ErrorHandler::CleanupArgs* , 
     dStorm::JobMaster* )
{
}

void rapidSTORM_Additional_Jobs( dStorm::JobMaster *job_master )
{
}

#ifdef __cplusplus
}
#endif
