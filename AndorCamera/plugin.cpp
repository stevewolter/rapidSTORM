#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dStorm/Config.h>
#include <dStorm/input/Link.h>
#include <dStorm/helpers/DisplayManager.h>

namespace dStorm {
namespace AndorCamera {

std::auto_ptr< dStorm::input::Link > get_method();

}
}


#ifdef __cplusplus
extern "C" {
#endif

const char * rapidSTORM_Plugin_Desc() {
    return "Direct camera input";
}

void rapidSTORM_Config_Augmenter ( dStorm::Config* config ) {
    std::auto_ptr< dStorm::input::Link > m = dStorm::AndorCamera::get_method();
    config->add_input( m, dStorm::InputMethod );
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
