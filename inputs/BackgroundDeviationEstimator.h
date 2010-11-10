#ifndef DSTORM_INPUT_BACKGROUND_STDDEV_ESTIMATOR_H
#define DSTORM_INPUT_BACKGROUND_STDDEV_ESTIMATOR_H

#include "BackgroundDeviationEstimator_decl.h"
#include <simparm/Object.hh>
#include <simparm/Structure.hh>
#include <dStorm/input/Source.h>
#include <dStorm/engine/Image.h>
#include <dStorm/input/chain/Filter.h>
#include <simparm/Entry.hh>
#include <boost/mpl/vector.hpp>

namespace dStorm {
namespace BackgroundStddevEstimator {

struct Config : public simparm::Object
{
    typedef boost::mpl::vector< dStorm::engine::Image > SupportedTypes;

    simparm::BoolEntry disable;
    Config();
    void registerNamedEntries() { push_back(disable); }
};

class Source 
: public input::Source<engine::Image>,
  public input::Filter,
  boost::noncopyable
{
    std::auto_ptr< input::Source<engine::Image> > base;
    int confidence_limit, binning;
  public:
    Source(std::auto_ptr< input::Source<engine::Image> > base);

    input::Source<engine::Image>::iterator begin() { return base->begin(); }
    input::Source<engine::Image>::iterator end() { return base->end(); }
    TraitsPtr get_traits();
    BaseSource& upstream() { return *base; }
    void dispatch(BaseSource::Messages m) { upstream().dispatch(m); }
};

class ChainLink
: public input::chain::Filter
{
    typedef input::chain::DefaultVisitor< Config > Visitor;
    friend class input::chain::DelegateToVisitor;
    simparm::Structure<Config>& get_config() { return config; }

    simparm::Structure<Config> config;
    bool do_make_source;
  public:
    ChainLink() : do_make_source(false) {}
    ChainLink* clone() const { return new ChainLink(*this); }

    AtEnd traits_changed( TraitsRef r, Link* l ) ;
    AtEnd context_changed( ContextRef c, Link* l );
    input::BaseSource* makeSource() ;

    simparm::Node& getNode() { return config; }
};

}
}

#endif
