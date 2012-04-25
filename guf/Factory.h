#ifndef DSTORM_FITTER_GUF_FACTORY_H
#define DSTORM_FITTER_GUF_FACTORY_H

#include "Config.h"
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/output/Traits.h>
#include <simparm/Structure.hh>

namespace dStorm {
namespace guf {

/** This class represents dStorm::guf::Fitter at configuration time.
 *  It is responsible for publishing meta information for the fields
 *  computed by the Fitter, and for registering appropriate configuration
 *  items. */
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
    void set_requirements( input::Traits<engine::ImageStack>& );
    void register_trait_changing_nodes( simparm::Listener& );
    void check_configuration( const engine::JobInfo& );
  private:
    bool can_do_3d( const input::Traits<engine::ImageStack>& ) const;
    bool can_compute_uncertainty( const engine::InputPlane& ) const;
    boost::optional<output::Traits> my_traits;
};

}
}

#endif
