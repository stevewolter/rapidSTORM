#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "reader.h"
#include "Output_decl.h"
#include <dStorm/Config.h>
#include <dStorm/helpers/DisplayManager.h>
#include <dStorm/output/OutputSource.h>

#ifdef __cplusplus
extern "C" {
#endif

const char * rapidSTORM_Plugin_Desc() {
    return PACKAGE_STRING;
}

void rapidSTORM_Config_Augmenter ( dStorm::Config* config ) {
    config->inputConfig.add_method( 
        new dStorm::localization_file::Reader::ChainLink() );
    config->outputConfig.addChoice( 
        dStorm::output::make_output_source< dStorm::localization_file::writer::Output >().release() );
    //config->inputConfig.add_filter( locprec::biplane_alignment::make_filter() );
    //config->outputConfig.addChoice( write_help_file( new locprec::SpotFinderEstimator::Source() ) );
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
