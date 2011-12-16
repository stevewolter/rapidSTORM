#ifndef DSTORM_INPUT_RESOLUTIONSETTER_H
#define DSTORM_INPUT_RESOLUTIONSETTER_H

#include "debug.h"
#include <dStorm/input/AdapterSource.h>
#include <dStorm/traits/resolution_config.h>

#include <simparm/TreeCallback.hh>
#include <dStorm/units/nanolength.h>
#include <simparm/Structure.hh>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Filter.h>

namespace dStorm {
namespace traits { namespace resolution { class Check; } }
namespace input {

using namespace chain;

namespace resolution {

class SourceConfig : public traits::resolution::Config {
    typedef input::chain::DefaultTypes SupportedTypes;
};

template <typename ForwardedType>
class Source 
: public input::AdapterSource<ForwardedType>
{
    SourceConfig config;

    void modify_traits( input::Traits<ForwardedType>& t )
        { config.set_traits(t); }
  public:
    Source(
        std::auto_ptr< input::Source<ForwardedType> > backend,
        const SourceConfig& config ) 
        : input::AdapterSource<ForwardedType>( backend ), config(config) {}
};


class ChainLink 
: public input::chain::Filter, public simparm::TreeListener 
{
    typedef input::chain::DefaultVisitor< SourceConfig > Visitor;
    friend class input::chain::DelegateToVisitor;
    friend class ::dStorm::traits::resolution::Check;

    simparm::Structure<SourceConfig> config;
    simparm::Structure<SourceConfig>& get_config() { return config; }
    ContextRef context;

    class TraitMaker;

  protected:
    void operator()(const simparm::Event&);

  public:
    ChainLink();
    ChainLink(const ChainLink&);
    ChainLink* clone() const { return new ChainLink(*this); }
    simparm::Node& getNode() { return config; }

    AtEnd traits_changed( TraitsRef r, Link* l);
    AtEnd context_changed( ContextRef r, Link* l);
    BaseSource* makeSource();
};

}

}
}

#endif
