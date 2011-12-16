#ifndef DSTORM_INPUT_SPLITTER_H
#define DSTORM_INPUT_SPLITTER_H

#include "Splitter_decl.h"
#include <simparm/Object.hh>
#include <simparm/Structure.hh>
#include <dStorm/input/AdapterSource.h>
#include <dStorm/engine/Image.h>
#include <dStorm/input/chain/Filter.h>
#include <simparm/Entry.hh>
#include <simparm/ChoiceEntry.hh>
#include <boost/mpl/vector.hpp>

namespace dStorm {
namespace Splitter {

struct Config : public simparm::Object
{
    typedef boost::mpl::vector< dStorm::engine::Image > SupportedTypes;
    enum Splits { Horizontal, Vertical, None };

    simparm::ChoiceEntry biplane_split;
    Config();
    void registerNamedEntries() { push_back(biplane_split); }
};

class Source 
: public input::AdapterSource<engine::Image>,
  boost::noncopyable
{
    struct iterator;
    const bool vertical;

    void modify_traits( input::Traits<engine::Image>& );
  public:
    Source(bool vertical, std::auto_ptr< input::Source<engine::Image> > base);

    input::Source<engine::Image>::iterator begin();
    input::Source<engine::Image>::iterator end();
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
