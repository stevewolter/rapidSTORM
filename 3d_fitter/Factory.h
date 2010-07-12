#ifndef DSTORM_3dGAUSSFITTER_H
#define DSTORM_3dGAUSSFITTER_H

#include <dStorm/engine/SpotFitterFactory.h>
#include "Config.h"
#include <simparm/Structure.hh>

namespace dStorm {
namespace gauss_3d_fitter {

template <int Widening>
class Factory 
: private simparm::Structure< Config<Widening> >, 
  public engine::SpotFitterFactory
{
  public:
    Factory();
    Factory(const Factory&);
    virtual ~Factory();

    std::auto_ptr<engine::SpotFitter> make( const engine::JobInfo& );
    Factory* clone() const { return new Factory(*this); }
    void set_traits( output::Traits& );
};

}
}

#endif
