#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dStorm/ModuleInterface.h>
#include "SegmentationFault.h"

using namespace dStorm::output;

OutputSource* write_help_file( OutputSource *src ) {
    src->help_file = "";
    return src;
}

#ifdef __cplusplus
extern "C" {
#endif

const char * rapidSTORM_Plugin_Desc() {
    return "Tests";
}

void rapidSTORM_Input_Augmenter ( dStorm::input::Config* inputs ) {
}

void rapidSTORM_Engine_Augmenter
    ( dStorm::engine::Config* config )
{
}

void rapidSTORM_Output_Augmenter
    ( dStorm::output::Config* outputs )
{
    outputs->addChoice( write_help_file( new SegmentationFault::Source() ) );
}


#ifdef __cplusplus
}
#endif
