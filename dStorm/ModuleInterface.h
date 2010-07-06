#ifndef DSTORM_MODULE_INTERFACE_H
#define DSTORM_MODULE_INTERFACE_H

#include <dStorm/Config_decl.h>
#include <dStorm/helpers/DisplayManager.h>
#include <dStorm/error_handler.h>
#include <dStorm/JobMaster.h>

typedef const char* (*RapidSTORM_Plugin_Desc) ();
typedef void (*RapidSTORM_Config_Augmenter)
    ( dStorm::Config* inputs );
typedef dStorm::Display::Manager* 
    (*RapidSTORM_Display_Driver)
    (dStorm::Display::Manager* current_manager);
typedef void
    (*RapidSTORM_Cleanup_Handler)
    (dStorm::ErrorHandler::CleanupArgs* current_args, 
     dStorm::JobMaster* job_master);

extern "C" {

const char* rapidSTORM_Plugin_Desc();
void rapidSTORM_Config_Augmenter
    ( dStorm::Config* inputs );
dStorm::Display::Manager* 
    rapidSTORM_Display_Driver
    (dStorm::Display::Manager* current_manager);
void
    rapidSTORM_Cleanup_Handler
    (dStorm::ErrorHandler::CleanupArgs* current_args, 
     dStorm::JobMaster* job_master);

}

#endif
