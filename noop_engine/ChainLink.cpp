#include "debug.h"

#include "ChainLink.h"
#include "Engine.h"
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Context_impl.h>
#include <dStorm/input/chain/Filter_impl.h>
#include <dStorm/input/chain/EngineHelpers.h>
#include <dStorm/output/LocalizedImage_traits.h>
#include <boost/mpl/for_each.hpp>

namespace dStorm {
namespace noop_engine {

struct ChainLink::Config {
    typedef boost::mpl::vector<dStorm::engine::Image> SupportedTypes;
    bool throw_errors;
    Config() : throw_errors(false) {}
};

ChainLink::Config ChainLink::get_config() { return Config(); }

class ChainLink::Visitor {
  public:
    typedef boost::mpl::vector<engine::Image>
        SupportedTypes;

    const Config& config;
    typedef std::auto_ptr< input::Source<output::LocalizedImage> > SourceResult;
    boost::shared_ptr<input::BaseTraits> new_traits;
    SourceResult new_source;

    Visitor(const Config& config ) : config(config) {}

    template <typename Type>
    bool operator()( input::Traits<Type>& source_traits ) { return true; }
    template <typename Type>
    bool operator()( const input::Traits<Type>& source_traits ) { 
        new_traits = Engine::convert_traits( source_traits ); 
        return true; 
    }
    template <typename Type>
    bool operator()( std::auto_ptr< input::Source< Type > > p )
        { new_source.reset( new Engine(p) ); return true; }

    bool operator()( input::chain::MetaInfo& ) { return true; }
    bool operator()( input::chain::Context& c ) {
        boost::mpl::for_each<SupportedTypes>( input::chain::ContextTraitCreator(c) );
        return true;
    }

    bool unknown_trait(std::string trait_desc) const {
        if ( config.throw_errors )
            throw std::runtime_error("No-op engine cannot work with input of type " + trait_desc);
        else
            return false;
    }
    bool no_context_visited_is_ok() const { return true; }
    void unknown_base_source() const {
        throw std::runtime_error("No-op engine cannot process input of the given type");
    }
};

ChainLink::ChainLink() 
: simparm::Object("Noop", "Do not localize")
{
}

input::BaseSource*
ChainLink::makeSource()
{
    return input::chain::DelegateToVisitor::makeSource(*this);
}

ChainLink::AtEnd
ChainLink::traits_changed(TraitsRef r, Link* l)
{
    return input::chain::DelegateToVisitor::traits_changed(*this,r,l);
}

ChainLink::AtEnd
ChainLink::context_changed(ContextRef r, Link* l)
{
    return input::chain::DelegateToVisitor::context_changed(*this,r,l);
}

std::auto_ptr<input::chain::Filter>
makeLink()
{
    return std::auto_ptr<input::chain::Filter>( new ChainLink( ) );
}

}
}
