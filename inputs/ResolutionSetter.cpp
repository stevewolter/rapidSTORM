#include "simparm/Eigen_decl.h"
#include "simparm/BoostUnits.h"

#include "inputs/ResolutionSetter.h"
#include "debug.h"

#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>

#include "engine/Image.h"
#include "helpers/make_unique.hpp"
#include "input/AdapterSource.h"
#include "input/InputMutex.h"
#include "input/Link.h"
#include "input/MetaInfo.h"
#include "input/FilterFactory.h"
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
: public input::AdapterSource<engine::ImageStack> {
    Config config;

    void modify_traits( input::Traits<engine::ImageStack>& t ) { 
        config.write_traits(t); 
        for (int p = 0; p < t.plane_count(); ++p)
            t.plane(p).create_projection();
    }
    void attach_local_ui_( simparm::NodeHandle ) {}

  public:
    Source(
        std::unique_ptr< input::Source<engine::ImageStack> > backend,
        const Config& config ) 
        : input::AdapterSource<engine::ImageStack>( std::move(backend) ), config(config) { 
            simparm::NodeHandle n = simparm::dummy_ui::make_node();
            this->config.attach_ui( n ); 
        }
};

class Factory 
: public input::FilterFactory<engine::ImageStack>
{
  public:
    Factory* clone() const OVERRIDE { return new Factory(*this); }

    void attach_ui(simparm::NodeHandle at,
                   std::function<void()> traits_change_callback) OVERRIDE {
        config.attach_ui( at );
        config.notify_on_any_change(traits_change_callback);
    }

    std::unique_ptr<input::Source<engine::ImageStack>>
    make_source(std::unique_ptr<input::Source<engine::ImageStack>> upstream) { 
        return make_unique<resolution::Source>(std::move(upstream), config); 
    }
    boost::shared_ptr<const Traits<engine::ImageStack>> make_meta_info(
        MetaInfo& meta_info,
        boost::shared_ptr<const Traits<engine::ImageStack>> traits) OVERRIDE { 
        auto mine = boost::make_shared<Traits<engine::ImageStack>>(*traits);
        config.set_context(*mine);
        config.write_traits(*mine); 
        return mine;
    }

  private:
    Config config;
};

std::unique_ptr<FilterFactory<engine::ImageStack>> create() {
    return make_unique<Factory>();
}

}
}
}

