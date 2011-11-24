#ifndef DSTORM_INPUT_RESOLUTIONSETTER_H
#define DSTORM_INPUT_RESOLUTIONSETTER_H

#include "debug.h"
#include <dStorm/input/Source.h>
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
: public input::Source<ForwardedType>, public input::Filter
{
    std::auto_ptr< input::Source<ForwardedType> > s;
    SourceConfig config;

  public:
    Source(
        std::auto_ptr< input::Source<ForwardedType> > backend,
        const SourceConfig& config ) 
        : input::Source<ForwardedType>( backend->getNode(), backend->flags ),
          s(backend), config(config) {}

    BaseSource& upstream() { return *s; }

    typedef typename input::Source<ForwardedType>::iterator iterator;
    typedef typename input::Source<ForwardedType>::TraitsPtr TraitsPtr;

    void dispatch(BaseSource::Messages m) { s->dispatch(m); }
    iterator begin() { return s->begin(); }
    iterator end() { return s->end(); }
    TraitsPtr get_traits();
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
