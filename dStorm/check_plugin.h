#ifndef DSTORM_MODULE_CHECKER_H
#define DSTORM_MODULE_CHECKER_H

#include "ModuleInterface.h"

namespace dStorm {

inline void check_plugin() {
    /* Only compile the code, do not run it. */
    return;
    dStorm::input::Config* i = NULL;
    dStorm::engine::Config* e = NULL;
    dStorm::output::Config* o = NULL;
    dStorm::Display::Manager* current_manager = NULL;
    dStorm::JobMaster* job_master = NULL;
    dStorm::ErrorHandler::CleanupArgs* args = NULL;
    rapidSTORM_Plugin_Desc();
    rapidSTORM_Input_Augmenter( i );
    rapidSTORM_Engine_Augmenter( e );
    rapidSTORM_Output_Augmenter( o );
    rapidSTORM_Display_Driver( current_manager );
    rapidSTORM_Additional_Jobs( job_master );
    rapidSTORM_Cleanup_Handler( args, job_master );
}

}

#endif
