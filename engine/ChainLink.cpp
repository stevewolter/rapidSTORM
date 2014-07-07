#include "engine/ChainLink.h"
#include "debug.h"

#include <boost/lexical_cast.hpp>
#include <boost/units/io.hpp>

#include "dejagnu.h"
#include "engine/ChainLink.h"
#include "engine/Engine.h"
#include "guf/Factory.h"
#include "helpers/make_unique.hpp"
#include "input/InputMutex.h"
#include "input/MetaInfo.h"
#include "input/Method.hpp"
#include "output/LocalizedImage_traits.h"
#include "spotFinders/spotFinders.h"

namespace dStorm {
namespace engine {

using namespace input;
using boost::signals2::scoped_connection;

class ChainLink
: public input::Method< ChainLink >
{
    friend class input::Method< ChainLink >;
    typedef boost::mpl::vector< engine::ImageStack > SupportedTypes;

    std::auto_ptr< scoped_connection > finder_con, fitter_con;
    Config config;
    simparm::Object engine_node;
    simparm::BaseAttribute::ConnectionStore listening[4];

    void notice_traits( const MetaInfo&, const Traits< ImageStack >& traits ) {
        config.separate_plane_fitting.set_visibility(traits.plane_count() > 1);
    }
    boost::shared_ptr< BaseTraits > 
    create_traits( MetaInfo& mi, 
                const Traits<ImageStack>& upstream )
    {
        boost::shared_ptr< input::Traits<output::LocalizedImage> >
            rt = Engine::convert_traits(config, upstream);
        update_meta_info( mi );
        return rt;
    }

    BaseSource* make_source( std::auto_ptr< Source<ImageStack> > base ) 
        { return new Engine( config, base ); }

    void republish_traits_locked() {
        input::InputMutexGuard lock( input::global_mutex() );
        republish_traits();
    }

    void add_spot_finder( std::unique_ptr<spot_finder::Factory> finder) 
        { config.spotFindingMethod.addChoice(std::move(finder)); }
    void add_spot_fitter( std::unique_ptr<spot_fitter::Factory> fitter) { 
        fitter->register_trait_changing_nodes(
            boost::bind( &ChainLink::republish_traits_locked, this ) );
        config.spotFittingMethod.addChoice(std::move(fitter)); 
    }

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    ~ChainLink() {}

    static std::string getName() { return "Engine"; }
    void attach_ui( simparm::NodeHandle at ) { 
        listening[0] = config.fit_judging_method.value.notify_on_value_change( 
            boost::bind( &ChainLink::republish_traits_locked, this ) );
        listening[1] = config.spotFittingMethod.value.notify_on_value_change( 
            boost::bind( &ChainLink::republish_traits_locked, this ) );
        listening[2] = config.spotFindingMethod.value.notify_on_value_change( 
            boost::bind( &ChainLink::republish_traits_locked, this ) );
        listening[3] = config.separate_plane_fitting.value.notify_on_value_change( 
            boost::bind( &ChainLink::republish_traits_locked, this ) );
        simparm::NodeHandle a = engine_node.attach_ui(at);
        config.attach_ui( a ); 
    }
};

ChainLink::ChainLink() 
: engine_node("Engine", "")
{
    add_spot_finder( spalttiefpass_smoother::make_spot_finder_factory() );
    add_spot_finder( median_smoother::make_spot_finder_factory() );
    add_spot_finder( erosion_smoother::make_spot_finder_factory() );
    add_spot_finder( gauss_smoother::make_spot_finder_factory() );
    add_spot_finder( spaltbandpass_smoother::make_spot_finder_factory() );
    add_spot_fitter( make_unique<guf::Factory>() );
}

ChainLink::ChainLink(const ChainLink& c)
: Method<ChainLink>(c),
  config(c.config),
  engine_node("Engine", "")
{
    for ( simparm::ManagedChoiceEntry< spot_fitter::Factory >::iterator 
            i = config.spotFittingMethod.begin(); 
            i != config.spotFittingMethod.end(); ++i )
    {
        i->register_trait_changing_nodes( 
            boost::bind( &ChainLink::republish_traits_locked, this )
        );
    }
}

std::auto_ptr<input::Link>
make_rapidSTORM_engine_link()
{
    std::auto_ptr<input::Link> rv( new ChainLink( ) );
    return rv;
}

void unit_test( TestState& state ) {
    std::auto_ptr<input::Link> rv = make_rapidSTORM_engine_link();
    rv.reset();
    state.pass("Destruction of engine works");
    std::auto_ptr<input::Link> cloner = make_rapidSTORM_engine_link();
    rv.reset( cloner->clone() );
    cloner.reset();
    rv.reset();
    state.pass("Destruction of engine works");
}

}
}
