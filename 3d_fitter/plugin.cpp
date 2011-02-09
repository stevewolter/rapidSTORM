#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "Factory.h"
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/ModuleInterface.h>
#include <dStorm/Config.h>
#include <simparm/ChoiceEntry_Impl.hh>

using namespace dStorm::gauss_3d_fitter;

#ifdef __cplusplus
extern "C" {
#endif

const char * rapidSTORM_Plugin_Desc() {
    return "Cylens fitter";
}

void rapidSTORM_Config_Augmenter ( dStorm::Config* inputs ) {
    inputs->add_spot_fitter( new Factory<fitpp::Exponential3D::Zhuang>() );
    inputs->add_spot_fitter( new Factory<fitpp::Exponential3D::Holtzer>() );
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
