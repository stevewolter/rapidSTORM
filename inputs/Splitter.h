#ifndef DSTORM_INPUT_SPLITTER_H
#define DSTORM_INPUT_SPLITTER_H

#include "Splitter_decl.h"
#include <simparm/Object.hh>
#include <simparm/Structure.hh>
#include <dStorm/input/Source.h>
#include <dStorm/engine/Image.h>
#include <dStorm/input/chain/Filter.h>
#include <simparm/Entry.hh>
#include <boost/mpl/vector.hpp>

namespace dStorm {
namespace Splitter {

struct Config : public simparm::Object
{
    typedef boost::mpl::vector< dStorm::engine::Image > SupportedTypes;

    simparm::BoolEntry enable;
    Config();
    void registerNamedEntries() { push_back(enable); }
};

class Source 
: public input::Source<engine::Image>,
  public input::Filter,
  boost::noncopyable
{
    std::auto_ptr< input::Source<engine::Image> > base;
    struct iterator;
  public:
    Source(std::auto_ptr< input::Source<engine::Image> > base);

    input::Source<engine::Image>::iterator begin();
    input::Source<engine::Image>::iterator end();
    TraitsPtr get_traits();
    BaseSource& upstream() { return *base; }
    void dispatch(BaseSource::Messages m) { upstream().dispatch(m); }
};

class ChainLink
: public input::chain::Filter, public simparm::Listener
{
    TraitsRef last_traits;
    typedef input::chain::DefaultVisitor< Config > Visitor;
    friend class input::chain::DelegateToVisitor;
    simparm::Structure<Config>& get_config() { return config; }

    simparm::Structure<Config> config;
    void operator()( const simparm::Event& );
  public:
    ChainLink();
    ChainLink(const ChainLink&);
    ChainLink* clone() const { return new ChainLink(*this); }

    AtEnd traits_changed( TraitsRef r, Link* l ) ;
    AtEnd context_changed( ContextRef c, Link* l );
    input::BaseSource* makeSource();

    simparm::Node& getNode() { return config; }
};

}
}

#endif
