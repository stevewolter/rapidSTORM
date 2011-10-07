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
#include <boost/shared_array.hpp>

namespace dStorm {
namespace BackgroundStddevEstimator {

struct Config : public simparm::Object
{
    typedef boost::mpl::vector< dStorm::engine::Image > SupportedTypes;

    simparm::BoolEntry enable;
    Config();
    void registerNamedEntries() { push_back(enable); }
};

struct lowest_histogram_mode_is_strongest : public std::runtime_error {
    lowest_histogram_mode_is_strongest() 
        : std::runtime_error("The lowest histogram pixel in the background estimation is the strongest") {}
};

struct ImagePlane {
    ImagePlane( int binning );
    void add_image( const Image<engine::StormPixel,2>& );
    bool converged() const;
    camera_response background_stddev() const;
  private:
    const int binning;
    int highest_bin, total_counts;
    boost::shared_array<int> histogram;
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
