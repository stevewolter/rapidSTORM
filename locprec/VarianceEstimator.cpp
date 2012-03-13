#include "VarianceEstimator.h"
#include <dStorm/output/OutputBuilder.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

namespace variance_estimator {

using namespace boost::accumulators;
using boost::units::quantity;
namespace si = boost::units::si;

class Output : public dStorm::output::OutputObject {
    typedef boost::accumulators::accumulator_set< double,
        stats< tag::count, tag::immediate_mean, tag::variance(immediate) > > Accumulator;
    Accumulator acc[3];
    class _Config;
    const std::string tag;

    void store_results_( bool success );

  public:
    typedef simparm::Structure<_Config> Config;

    Output( const Config& );
    Output* clone() const;
    AdditionalData announceStormSize(const Announcement&);
    RunRequirements announce_run(const RunAnnouncement&) { 
        for (int j = 0; j < 3; ++j) acc[j] = Accumulator();
        return RunRequirements(); 
    }
    void receiveLocalizations(const EngineResult&);

    void check_for_duplicate_filenames
            (std::set<std::string>&) { }
};

struct Output::_Config 
: public simparm::Object
{
    simparm::Entry<std::string> tag;
  public:
    _Config() 
        : simparm::Object("VarianceEstimator", "Estimate variance"),
          tag("Tag", "Tag at start of line", "Precision") {}
    void registerNamedEntries() { userLevel = simparm::Object::Expert; push_back(tag); }
    bool can_work_with( dStorm::output::Capabilities ) { return true; }
};

Output* Output::clone() const { return new Output(*this); }

Output::Output( const Config& config ) 
: OutputObject("VarianceEstimator", "Estimate variance"),
  tag(config.tag())
{
}

Output::AdditionalData Output::announceStormSize(const Announcement& a) {
    return Output::AdditionalData();
}

void Output::receiveLocalizations(const EngineResult& e) {
    for ( EngineResult::const_iterator i = e.begin(); i != e.end(); ++i ) {
        for (int j = 0; j < 2; ++j)
            acc[j]( i->position()[j].value() );
        acc[2]( i->amplitude().value() );
    }
}

void Output::store_results_( bool success ) {
    if ( success ) {
        std::cout << tag;
        for (int j = 0; j < 3; ++j) 
            std::cout << " " << count( acc[j] ) << " " << mean( acc[j] ) << " " << sqrt( variance(acc[j]) );
        std::cout << "\n";
    }
}

}

namespace dStorm {
namespace output {

template <>
std::auto_ptr<OutputSource> make_output_source<variance_estimator::Output>()
{
    return std::auto_ptr<OutputSource>( new dStorm::output::OutputBuilder<variance_estimator::Output>() );
}

}
}
