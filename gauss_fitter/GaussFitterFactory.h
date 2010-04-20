#ifndef DSTORM_ENGINE_GAUSSFITTER_H
#define DSTORM_ENGINE_GAUSSFITTER_H

#include <dStorm/engine/SpotFitterFactory.h>
#include "GaussFitterConfig.h"
#include <simparm/Structure.hh>

namespace dStorm {
namespace engine {

class GaussFitterFactory 
: private simparm::Structure<GaussFitterConfig>, public SpotFitterFactory
{
  public:
    GaussFitterFactory();

    std::auto_ptr<SpotFitter> make( const JobInfo& );
    GaussFitterFactory* clone() const { return new GaussFitterFactory(*this); }
};

}
}

#endif
