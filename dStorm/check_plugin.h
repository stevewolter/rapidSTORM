#ifndef DSTORM_MODULE_CHECKER_H
#define DSTORM_MODULE_CHECKER_H

#include "ModuleInterface.h"

namespace dStorm {

inline void check_plugin() {
    /* Only compile the code, do not run it. */
    return;
    dStorm::Config* e = NULL;
    dStorm::Display::Manager* current_manager = NULL;
    dStorm::JobMaster* job_master = NULL;
    dStorm::ErrorHandler::CleanupArgs* args = NULL;
    rapidSTORM_Plugin_Desc();
    rapidSTORM_Config_Augmenter( e );
    rapidSTORM_Display_Driver( current_manager );
    rapidSTORM_Cleanup_Handler( args, job_master );
}

}

#endif
