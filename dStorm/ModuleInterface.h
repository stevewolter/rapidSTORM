#ifndef DSTORM_MODULE_INTERFACE_H
#define DSTORM_MODULE_INTERFACE_H

#include "Config_decl.h"
#include "display/Manager.h"
#include "JobMaster.h"

typedef const char* (*RapidSTORM_Plugin_Desc) ();
typedef void (*RapidSTORM_Config_Augmenter)
    ( dStorm::Config* inputs );
typedef dStorm::display::Manager* 
    (*RapidSTORM_Display_Driver)
    (dStorm::display::Manager* current_manager);

extern "C" {

const char* rapidSTORM_Plugin_Desc();
void rapidSTORM_Config_Augmenter
    ( dStorm::Config* inputs );
dStorm::display::Manager* 
    rapidSTORM_Display_Driver
    (dStorm::display::Manager* current_manager);

}

#endif
