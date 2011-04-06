#ifndef DSTORM_3dGAUSSFITTER_H
#define DSTORM_3dGAUSSFITTER_H

#include "Config.h"
#include <dStorm/engine/SpotFitterFactory.h>
#include <simparm/Structure.hh>

namespace dStorm {
namespace gauss_3d_fitter {

template <int Widening>
class Factory 
: private simparm::Structure< Config<Widening> >, 
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
