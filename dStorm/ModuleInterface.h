#ifndef DSTORM_MODULE_INTERFACE_H
#define DSTORM_MODULE_INTERFACE_H

#include <dStorm/engine/Config.h>
#include <dStorm/input/Config.h>
#include <dStorm/output/Config.h>
#include <dStorm/helpers/DisplayManager.h>

typedef const char* (*RapidSTORM_Plugin_Desc) ();
typedef void (*RapidSTORM_Input_Augmenter)
    ( dStorm::input::Config* inputs );
typedef void (*RapidSTORM_Engine_Augmenter)
    ( dStorm::engine::Config* config );
typedef void (*RapidSTORM_Output_Augmenter)
    ( dStorm::output::Config* outputs );
typedef dStorm::Display::Manager* 
    (*RapidSTORM_Display_Driver)
    (dStorm::Display::Manager* current_manager);

#endif
