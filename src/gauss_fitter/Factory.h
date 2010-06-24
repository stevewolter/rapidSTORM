#ifndef DSTORM_ENGINE_GAUSSFITTER_H
#define DSTORM_ENGINE_GAUSSFITTER_H

#include <dStorm/engine/SpotFitterFactory.h>
#include "Config.h"
#include <simparm/Structure.hh>

namespace dStorm {
namespace 2d_fitter {

class Factory 
: private simparm::Structure<Config>, public SpotFitterFactory
{
  public:
    Factory();
    Factory(const Factory&);
    virtual ~Factory();

    std::auto_ptr<SpotFitter> make( const JobInfo& );
    Factory* clone() const { return new Factory(*this); }
    void set_traits( output::Traits& );
};

}
}

#endif
