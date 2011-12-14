#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/ModuleInterface.h>
#include <dStorm/Config.h>
#include "AndorCamera/InputChainLink.h"

using namespace dStorm::output;

#ifdef __cplusplus
extern "C" {
#endif

const char * rapidSTORM_Plugin_Desc() {
    return "Direct camera input";
}

void rapidSTORM_Config_Augmenter ( dStorm::Config* config ) {
    config->inputConfig.add_method( new dStorm::AndorCamera::Method(), dStorm::input::chain::Link::InputMethod );
}

dStorm::Display::Manager*
rapidSTORM_Display_Driver
    (dStorm::Display::Manager *old)
{
    return old;
}

#ifdef __cplusplus
}
#endif
