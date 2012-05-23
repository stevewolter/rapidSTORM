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
#include <boost/lexical_cast.hpp>
#include "dejagnu.h"

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
    simparm::BaseAttribute::ConnectionStore listening[4];

    void notice_traits( const MetaInfo&, const Traits< ImageStack >& traits ) {
        int soll = traits.plane_count() - 1;
        while ( soll > int(config.spot_finder_weights.size()) ) {
            int id = config.spot_finder_weights.size();
            std::string ident = boost::lexical_cast<std::string>(id + 1);
            config.spot_finder_weights.push_back( 
                new simparm::Entry<float>(
                    "WeightInPlane" + boost::lexical_cast<std::string>(id + 1),
                    "Input layer " + boost::lexical_cast<std::string>(id + 2),
                    1.0f
                )
            );
            if ( config.weights_insertion_point )
                config.spot_finder_weights.back().attach_ui( *config.weights_insertion_point );
        }

        for (int i = 0; i < int(config.spot_finder_weights.size()); ++i)
            config.spot_finder_weights[i].viewable = i < soll;
        config.weights.viewable = soll > 0;
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

    BaseSource* make_source( std::auto_ptr< Source<ImageStack> > base ) 
        { return new Engine( config, base ); }

    std::string amplitude_threshold_string() const;

    void republish_traits_locked() {
        input::InputMutexGuard lock( input::global_mutex() );
        config.amplitude_threshold.viewable = ! config.guess_threshold();
        config.threshold_height_factor.viewable = config.guess_threshold();
        republish_traits();
    }

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    ~ChainLink() {}

    static std::string getName() { return "rapidSTORM"; }
    void attach_ui( simparm::Node& at ) { 
        listening[0] = config.guess_threshold.value.notify_on_value_change( 
            boost::bind( &ChainLink::republish_traits_locked, this ) );
        listening[1] = config.amplitude_threshold.value.notify_on_value_change( 
            boost::bind( &ChainLink::republish_traits_locked, this ) );
        listening[2] = config.spotFittingMethod.value.notify_on_value_change( 
            boost::bind( &ChainLink::republish_traits_locked, this ) );
        listening[3] = config.spotFindingMethod.value.notify_on_value_change( 
            boost::bind( &ChainLink::republish_traits_locked, this ) );
        config.attach_ui( at ); 
    }

    void add_spot_finder( const spot_finder::Factory& finder) 
        { config.spotFindingMethod.addChoice(finder.clone()); }
    void add_spot_fitter( const spot_fitter::Factory& fitter) { 
        std::auto_ptr< spot_fitter::Factory > my_fitter(fitter.clone());
        my_fitter->register_trait_changing_nodes(
            boost::bind( &ChainLink::republish_traits_locked, this ) );
        config.spotFittingMethod.addChoice(my_fitter); 
    }
};

ChainLink::ChainLink() 
{
}

ChainLink::ChainLink(const ChainLink& c)
: Method<ChainLink>(c),
  config(c.config)
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

std::string ChainLink::amplitude_threshold_string() const
{
    std::stringstream ss; 
    if ( config.guess_threshold() )
        ss << "auto";
    else 
        ss << config.amplitude_threshold();
    return ss.str();
}

std::auto_ptr<input::Link>
make_rapidSTORM_engine_link()
{
    std::auto_ptr<input::Link> rv( new ChainLink( ) );
    return rv;
}

void check_plane_flattener( TestState& state );

void unit_test( TestState& state ) {
    check_plane_flattener( state );

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
