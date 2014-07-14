#include "simparm/Eigen_decl.h"
#include "simparm/BoostUnits.h"

#include "inputs/ResolutionSetter.h"
#include "debug.h"

#include <boost/lexical_cast.hpp>

#include "engine/Image.h"
#include "input/AdapterSource.h"
#include "input/InputMutex.h"
#include "input/Link.h"
#include "input/MetaInfo.h"
#include "input/Method.hpp"
#include "input/Source.h"
#include "Localization.h"
#include "localization/Traits.h"
#include "simparm/dummy_ui/fwd.h"
#include "simparm/Eigen.h"
#include "traits/optics_config.h"

namespace dStorm {
namespace input {
namespace resolution {

struct Config : public traits::MultiPlaneConfig {
    Config() : traits::MultiPlaneConfig( traits::PlaneConfig::FitterConfiguration ) {}
};

class Source 
: public input::AdapterSource<engine::ImageStack>
{
    Config config;

    void modify_traits( input::Traits<engine::ImageStack>& t ) { 
        config.write_traits(t); 
        for (int p = 0; p < t.plane_count(); ++p)
            t.plane(p).create_projection();
    }
    void attach_local_ui_( simparm::NodeHandle ) {}

  public:
    Source(
        std::auto_ptr< input::Source<engine::ImageStack> > backend,
        const Config& config ) 
        : input::AdapterSource<engine::ImageStack>( backend ), config(config) { 
            simparm::NodeHandle n = simparm::dummy_ui::make_node();
            this->config.attach_ui( n ); 
        }
};

class ChainLink 
: public input::Method<ChainLink>
{
    friend class Check;
    friend class input::Method<ChainLink>;

    Config config;
    void attach_ui( simparm::NodeHandle at ) { config.attach_ui( at ); }
    static std::string getName() { return "Optics"; }

    input::Source<output::LocalizedImage>* make_source( std::auto_ptr< input::Source<output::LocalizedImage> > upstream ) { 
        return upstream.release();
    }
    input::Source<engine::ImageStack>* make_source( std::auto_ptr< input::Source<engine::ImageStack> > upstream ) { 
        return new resolution::Source(upstream, config); 
    }
    void update_traits( MetaInfo& i, Traits<output::LocalizedImage>& traits ) {}
    void update_traits( MetaInfo& i, Traits<engine::ImageStack>& traits ) { 
        config.set_context( traits );
        config.write_traits(traits); 
    }
    void republish_traits();

  public:
    ChainLink();
    ChainLink(const ChainLink&);
};

ChainLink::ChainLink() 
{
    config.notify_on_any_change( boost::bind( &ChainLink::republish_traits, this ) );
}

ChainLink::ChainLink(const ChainLink& o) 
: input::Method<ChainLink>(o),
  config(o.config)
{
    config.notify_on_any_change( boost::bind( &ChainLink::republish_traits, this ) );
}

void ChainLink::republish_traits()
{
    InputMutexGuard lock( global_mutex() );
    input::Method<ChainLink>::republish_traits();
}

std::auto_ptr<Link> makeLink() {
    DEBUG("Making resolution chain link");
    return std::auto_ptr<Link>( new ChainLink() );
}

}
}
}

