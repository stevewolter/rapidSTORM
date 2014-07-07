#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "engine/SpotFinder.h"
#include "engine/SpotFitterFactory.h"
#include "base/Config.h"
#include "output/OutputSource.h"
#include "guf/Factory.h"

namespace dStorm {
namespace guf {

void augment_config ( dStorm::Config& inputs ) 
{
    inputs.add_spot_fitter( new dStorm::guf::Factory() );
}

}
}
