#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <dStorm/engine/SpotFinder.h>
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/ModuleInterface.h>
#include <dStorm/Config.h>
#include <dStorm/output/OutputSource.h>
#include <simparm/ChoiceEntry_Impl.hh>
#include "form_fitter/decl.h"
#include "guf/Factory.h"

namespace dStorm {
namespace guf {

void augment_config ( dStorm::Config& inputs ) 
{
    inputs.add_spot_fitter( new dStorm::guf::Factory() );
    inputs.add_output( dStorm::output::make_output_source<dStorm::form_fitter::Output>() );
}

}
}
