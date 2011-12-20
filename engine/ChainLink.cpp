#include "debug.h"

#include "ChainLink.h"
#include "Engine.h"
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <boost/units/io.hpp>
#include <dStorm/input/InputMutex.h>
#include <dStorm/input/Method.hpp>

namespace dStorm {
namespace engine {

using namespace input;

class ChainLink
: protected simparm::Listener,
  public input::Method< ChainLink, ClassicEngine >
{
    friend class input::Method< ChainLink, ClassicEngine >;
    typedef boost::mpl::vector< engine::Image > SupportedTypes;

    boost::shared_ptr< BaseTraits > 
    create_traits( chain::MetaInfo& mi, 
                const Traits<Image>& upstream )
    {
        boost::shared_ptr< input::Traits<output::LocalizedImage> >
            rt = Engine::convert_traits(config, upstream);
        mi.suggested_output_basename.set_variable
            ( "thres", amplitude_threshold_string() );
        return rt;
    }
    BaseSource* make_source( std::auto_ptr< Source<Image> > base ) 
        { return new Engine( config, base ); }
    Config config;

    std::string amplitude_threshold_string() const;

  protected:
    void operator()( const simparm::Event& ) {
        ost::MutexLock lock( input::global_mutex() );
        republish_traits();
    }

  public:
    ChainLink();
    ChainLink(const ChainLink&);

    simparm::Node& getNode() { return config; }

    void add_spot_finder( spot_finder::Factory& finder) { config.spotFindingMethod.addChoice(finder); }
    void add_spot_fitter( spot_fitter::Factory& fitter) { 
        fitter.register_trait_changing_nodes(*this);
        config.spotFittingMethod.addChoice(fitter); 
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
  Method<ChainLink,ClassicEngine>(c),
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

std::auto_ptr<input::chain::Link>
make_rapidSTORM_engine_link()
{
    return std::auto_ptr<input::chain::Link>( new ChainLink( ) );
}

}
}
