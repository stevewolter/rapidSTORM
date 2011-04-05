#ifndef DSTORM_ENGINE_GAUSSFITTER_H
#define DSTORM_ENGINE_GAUSSFITTER_H

#include <dStorm/engine/SpotFitterFactory.h>
#include "Config.h"
#include <simparm/Structure.hh>

namespace dStorm {
namespace gauss_2d_fitter {

class Factory 
: private simparm::Structure<Config>, 
  public engine::spot_fitter::Factory
{
  public:
    Factory();
    Factory(const Factory&);
    virtual ~Factory();

    std::auto_ptr<engine::spot_fitter::Implementation> make( const engine::JobInfo& );
    Factory* clone() const { return new Factory(*this); }
    void set_traits( output::Traits&, const engine::JobInfo& );
    void set_requirements( input::Traits<engine::Image>& );
};

}
}

#endif
