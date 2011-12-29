#include "debug.h"

#include "ChainLink.h"
#include "Engine.h"
#include <dStorm/input/MetaInfo.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <boost/units/io.hpp>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/Method.hpp>
#include <dStorm/signals/UseSpotFinder.h>
#include <dStorm/signals/UseSpotFitter.h>
#include "dejagnu.h"

namespace dStorm {
namespace engine {

using namespace input;
using boost::signals2::scoped_connection;

class ChainLink
: protected simparm::Listener,
  public input::Method< ChainLink >
{
    friend class input::Method< ChainLink >;
    typedef boost::mpl::vector< engine::Image > SupportedTypes;

    std::auto_ptr< scoped_connection > finder_con, fitter_con;

    boost::shared_ptr< BaseTraits > 
    create_traits( MetaInfo& mi, 
                const Traits<Image>& upstream )
    {
        boost::shared_ptr< input::Traits<output::LocalizedImage> >
            rt = Engine::convert_traits(config, upstream);
        update_meta_info( mi );
        return rt;
    }
    void update_meta_info( MetaInfo& mi ) {
        mi.suggested_output_basename.set_variable
            ( "thres", amplitude_threshold_string() );
        finder_con.reset( new scoped_connection( 
            mi.get_signal< signals::UseSpotFinder >().connect( 
                boost::bind( &ChainLink::add_spot_finder, this, _1 ) ) ) );
        fitter_con.reset( new scoped_connection( 
            mi.get_signal< signals::UseSpotFitter >().connect( 
                boost::bind( &ChainLink::add_spot_fitter, this, _1 ) ) ) );
    }

    BaseSource* make_source( std::auto_ptr< Source<Image> > base ) 
        { return new Engine( config, base ); }
    Config config;

    std::string amplitude_threshold_string() const;

  protected:
    void operator()( const simparm::Event& ) {
        boost::lock_guard<boost::mutex> lock( input::global_mutex() );
        republish_traits();
    }

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    ~ChainLink() {}

    simparm::Node& getNode() { return config; }

    void add_spot_finder( const spot_finder::Factory& finder) 
        { config.spotFindingMethod.addChoice(finder.clone()); }
    void add_spot_fitter( const spot_fitter::Factory& fitter) { 
        std::auto_ptr< spot_fitter::Factory > my_fitter(fitter.clone());
        my_fitter->register_trait_changing_nodes(*this);
        config.spotFittingMethod.addChoice(my_fitter); 
    }
};

ChainLink::ChainLink() 
: simparm::Listener( simparm::Event::ValueChanged )
{
    receive_changes_from( config.amplitude_threshold.value );
    receive_changes_from( config.spotFittingMethod.value );
    receive_changes_from( config.spotFindingMethod.value );
}

ChainLink::ChainLink(const ChainLink& c)
: simparm::Listener( simparm::Event::ValueChanged ),
  Method<ChainLink>(c),
  config(c.config)
{
    receive_changes_from( config.amplitude_threshold.value );
    receive_changes_from( config.spotFittingMethod.value );
    receive_changes_from( config.spotFindingMethod.value );

    for ( simparm::NodeChoiceEntry< spot_fitter::Factory >::iterator 
            i = config.spotFittingMethod.beginChoices(); 
            i != config.spotFittingMethod.endChoices(); ++i )
    {
        i->register_trait_changing_nodes(*this);
    }
}

std::string ChainLink::amplitude_threshold_string() const
{
    std::stringstream ss; 
    if ( config.amplitude_threshold().is_initialized() )
        ss << *config.amplitude_threshold();
    else 
        ss << "auto";
    return ss.str();
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
    rv.reset( make_rapidSTORM_engine_link()->clone() );
    rv.reset();
    state.pass("Destruction of engine works");
}

}
}
