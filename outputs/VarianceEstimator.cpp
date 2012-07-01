#include "VarianceEstimator.h"
#include <dStorm/output/OutputBuilder.h>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp>

namespace dStorm { 
namespace outputs {
namespace variance_estimator {

using namespace boost::accumulators;
using boost::units::quantity;
namespace si = boost::units::si;

struct Config 
{
    simparm::Entry<std::string> tag;
  public:
    static std::string get_name() { return "VarianceEstimator"; }
    static std::string get_description() { return "Estimate localization precision naively"; }
    static simparm::UserLevel get_user_level() { return simparm::Intermediate; }
    Config() 
        : tag("Tag", "Tag at start of line", "Precision") {}
    void attach_ui( simparm::NodeHandle at ) { tag.attach_ui(at); }
    bool can_work_with( dStorm::output::Capabilities ) { return true; }
};

class Output : public dStorm::output::Output {
    typedef boost::accumulators::accumulator_set< double,
        stats< tag::count, tag::immediate_mean, tag::variance(immediate) > > Accumulator;
    Accumulator acc[3];
    const std::string tag;

    void store_results_( bool success );

  public:
    Output( const Config& );
    AdditionalData announceStormSize(const Announcement&);
    RunRequirements announce_run(const RunAnnouncement&) { 
        for (int j = 0; j < 3; ++j) acc[j] = Accumulator();
        return RunRequirements(); 
    }
    void receiveLocalizations(const EngineResult&);

    void check_for_duplicate_filenames
            (std::set<std::string>&) { }
};

Output::Output( const Config& config ) 
: tag(config.tag())
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

std::auto_ptr<dStorm::output::OutputSource> make_variance_estimator_source()
{
    return std::auto_ptr<dStorm::output::OutputSource>( new dStorm::output::OutputBuilder<variance_estimator::Config,variance_estimator::Output>() );
}

}
}
