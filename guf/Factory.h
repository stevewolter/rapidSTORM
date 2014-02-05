#ifndef DSTORM_FITTER_GUF_FACTORY_H
#define DSTORM_FITTER_GUF_FACTORY_H

#include "guf/Config.h"
#include <dStorm/engine/SpotFitterFactory.h>
#include <dStorm/output/Traits.h>
#include <dStorm/helpers/default_on_copy.h>

namespace dStorm {
namespace guf {

/** This class represents dStorm::guf::Fitter at configuration time.
 *  It is responsible for publishing meta information for the fields
 *  computed by the Fitter, and for registering appropriate configuration
 *  items. */
struct Factory
: public engine::spot_fitter::Factory
{
    Config config;

    std::auto_ptr<engine::spot_fitter::Implementation> make( const engine::JobInfo& );
    Factory* clone() const { return new Factory(*this); }
    void set_traits( output::Traits&, const engine::JobInfo& );
    void set_requirements( input::Traits<engine::ImageStack>& );
    void register_trait_changing_nodes( simparm::BaseAttribute::Listener );
    void check_configuration( const engine::JobInfo& );
  private:
    bool can_do_3d( const input::Traits<engine::ImageStack>& ) const;
    bool can_compute_uncertainty( const engine::InputPlane& ) const;
    boost::optional<output::Traits> my_traits;
    default_on_copy< boost::signals2::signal<void()> > traits_changed;

    std::string getName() const { return Config::getName(); }
    void attach_ui( simparm::NodeHandle to );
    void detach_ui( simparm::NodeHandle to ) { throw std::logic_error("Removing spot fitter choices is not supported"); }

    simparm::BaseAttribute::ConnectionStore listening[4];

};

}
}

#endif
