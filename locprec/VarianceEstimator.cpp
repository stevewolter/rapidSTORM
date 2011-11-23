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
        stats< tag::immediate_mean, tag::variance(immediate) > > Accumulator;
    Accumulator acc[2];
    class _Config;

  public:
    typedef simparm::Structure<_Config> Config;

    Output( const Config& );
    Output* clone() const;
    AdditionalData announceStormSize(const Announcement&);
    void propagate_signal(ProgressSignal);
    Result receiveLocalizations(const EngineResult&);

    void check_for_duplicate_filenames
            (std::set<std::string>&) { }
};

struct Output::_Config 
: public simparm::Object
{
  public:
    _Config() 
        : simparm::Object("VarianceEstimator", "Estimate variance") {}
    void registerNamedEntries() { userLevel = simparm::Object::Expert; }
    bool can_work_with( dStorm::output::Capabilities ) { return true; }
};

Output* Output::clone() const { return new Output(*this); }

Output::Output( const Config& config ) 
: OutputObject("VarianceEstimator", "Estimate variance")
{
}

Output::AdditionalData Output::announceStormSize(const Announcement& a) {
    return Output::AdditionalData();
}

Output::Result Output::receiveLocalizations(const EngineResult& e) {
    for ( EngineResult::const_iterator i = e.begin(); i != e.end(); ++i ) {
        for (int j = 0; j < 2; ++j)
            acc[j]( i->position()[j].value() );
    }
    return KeepRunning;
}

void Output::propagate_signal(ProgressSignal e) {
    if ( e == Engine_is_restarted ) {
        for (int j = 0; j < 2; ++j) acc[j] = Accumulator();
    } else if ( e == Engine_run_succeeded  ) {
        for (int j = 0; j < 2; ++j) 
            std::cerr << mean( acc[j] ) << " " << sqrt( variance(acc[j]) ) << std::endl;
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
