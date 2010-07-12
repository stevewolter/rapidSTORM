#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dStorm/ModuleInterface.h>
#include <dStorm/Config.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include "Factory.h"

using namespace dStorm::gauss_3d_fitter;

#ifdef __cplusplus
extern "C" {
#endif

const char * rapidSTORM_Plugin_Desc() {
    return "Cylens fitter";
}

void rapidSTORM_Config_Augmenter ( dStorm::Config* inputs ) {
    inputs->engineConfig.spotFittingMethod.addChoice( new Factory<fitpp::Exponential3D::Zhuang>() );
    inputs->engineConfig.spotFittingMethod.addChoice( new Factory<fitpp::Exponential3D::Holtzer>() );
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

#ifdef __cplusplus
}
#endif
