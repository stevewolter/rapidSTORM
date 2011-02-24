#ifndef DSTORM_INPUT_YMIRROR_H
#define DSTORM_INPUT_YMIRROR_H

#include "YMirror_decl.h"
#include <simparm/Object.hh>
#include <simparm/Structure.hh>
#include <dStorm/input/Source.h>
#include <dStorm/Image.h>
#include <dStorm/input/chain/Filter.h>
#include <simparm/Entry.hh>
#include <dStorm/input/chain/DefaultFilterTypes.h>

namespace dStorm {
namespace YMirror {

struct Config : public simparm::Object
{
    typedef input::chain::DefaultTypes SupportedTypes;

    simparm::BoolEntry mirror_y;
    Config();
    void registerNamedEntries() { push_back(mirror_y); }
};

template <typename Type>
class Source
: public input::Source< Type >,
  public input::Filter,
  boost::noncopyable
{
    typedef Type Input;
    typedef input::Source<Input> Base;
    std::auto_ptr< Base > base;
    typedef Localization::Position::Traits::RangeType::Scalar Range;
    Range range;
    struct iterator;

  public:
    Source( std::auto_ptr< Base > base ) 
        : Base(base->getNode(), base->flags), base(base) {}
    typename Base::iterator begin();
    typename Base::iterator end();
    typename Base::TraitsPtr get_traits();
    input::BaseSource& upstream() { return *base; }
    void dispatch(input::BaseSource::Messages m) { upstream().dispatch(m); }
};

class ChainLink
: public input::chain::Filter
{
    typedef input::chain::DefaultVisitor< Config > Visitor;
    friend class input::chain::DelegateToVisitor;
    simparm::Structure<Config>& get_config() { return config; }

    simparm::Structure<Config> config;
  public:
    ChainLink() {}
    ChainLink* clone() const { return new ChainLink(*this); }

    AtEnd traits_changed( TraitsRef r, Link* l ) ;
    AtEnd context_changed( ContextRef c, Link* l );
    input::BaseSource* makeSource() ;

    simparm::Node& getNode() { return config; }
};

}
}

#endif
