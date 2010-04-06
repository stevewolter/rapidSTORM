#include "SpotFitterFactory.h"
#include "JobInfo.h"
#include "SpotFitter.h"

namespace dStorm {
namespace engine {

std::auto_ptr<SpotFitter>
SpotFitterFactory::make_by_parts( const Config& c, const InputTraits& i) 
{
    return make( JobInfo(c,i) );
}

SpotFitterFactory::~SpotFitterFactory() {}

}
}
