#ifndef DSTORM_FITTER_GUF_FACTORY_H
#define DSTORM_FITTER_GUF_FACTORY_H

#include "Config.h"
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/output/Traits.h>
#include <simparm/Structure.hh>

namespace dStorm {
namespace guf {

struct Factory
: private simparm::Structure< Config >,
  public engine::spot_fitter::Factory
{
    Factory();
    Factory(const Factory&);
    ~Factory();

    std::auto_ptr<engine::spot_fitter::Implementation> make( const engine::JobInfo& );
    Factory* clone() const { return new Factory(*this); }
    void set_traits( output::Traits&, const engine::JobInfo& );
    void set_requirements( input::Traits<engine::Image>& );
    void register_trait_changing_nodes( simparm::Listener& );
    void check_configuration( const engine::JobInfo& );
  private:
    bool can_do_3d( const input::Traits<engine::Image>& ) const;
    bool can_compute_uncertainty( const traits::Optics<2>& ) const;
    boost::optional<output::Traits> my_traits;
};

}
}

#endif
