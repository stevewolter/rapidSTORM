#ifndef DSTORM_MODULE_CHECKER_H
#define DSTORM_MODULE_CHECKER_H

#include "ModuleInterface.h"

namespace dStorm {

inline void check_plugin() {
    /* Only compile the code, do not run it. */
    return;
    dStorm::Config* e = NULL;
    dStorm::display::Manager* current_manager = NULL;
    rapidSTORM_Plugin_Desc();
    rapidSTORM_Config_Augmenter( e );
    rapidSTORM_Display_Driver( current_manager );
}

}

#endif
