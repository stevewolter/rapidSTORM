#include "reader.h"
#include "Output_decl.h"
#include <dStorm/Config.h>
#include <dStorm/helpers/DisplayManager.h>
#include <dStorm/output/OutputSource.h>

#ifdef __cplusplus
extern "C" {
#endif

const char * rapidSTORM_Plugin_Desc() {
    return "";
}

void rapidSTORM_Config_Augmenter ( dStorm::Config* config ) {
    config->add_input( 
        new dStorm::localization_file::Reader::ChainLink(),
        dStorm::FileReader );
    config->add_output( 
        dStorm::output::make_output_source< dStorm::localization_file::writer::Output >() );
}

dStorm::Display::Manager*
rapidSTORM_Display_Driver
    (dStorm::Display::Manager *current)
{
    return current;
}

#ifdef __cplusplus
}
#endif
