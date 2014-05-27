#include "outputs/TraceFilter.h"

#include <vector>
#include <numeric>
#include <boost/bind/bind.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include "base/Engine.h"
#include "binning/binning.h"
#include "binning/config.h"
#include "output/Filter.h"
#include "output/FilterBuilder.h"
#include "simparm/ChoiceEntry.h"
#include "simparm/Entry.h"

namespace dStorm {
using namespace output;
namespace outputs {
namespace {

class TraceCountConfig
{
  public:
    simparm::Entry<unsigned long> min_count;

    TraceCountConfig();

    void attach_ui( simparm::NodeHandle at );
    static std::string get_name() { return "TraceFilter"; }
    static std::string get_description() { return "Trace filter"; }
    static simparm::UserLevel get_user_level() { return simparm::Intermediate; }

    bool determine_output_capabilities( output::Capabilities& cap ) {
        return true;
    }
};

TraceCountConfig::TraceCountConfig()
: min_count("MinEmissionCount",
            "Minimum number of emissions per trace", 0) {}

void TraceCountConfig::attach_ui( simparm::NodeHandle at )
{
    min_count.attach_ui(at);
}

class TraceCountFilter : public output::Filter
{
  private:
    const int min_count;

    AdditionalData announceStormSize(const Announcement &a) {
        if (a.group_field == input::GroupFieldSemantic::ImageNumber) {
            throw std::runtime_error("Input to trace count filter is not sorted");
        }
        return output::Filter::announceStormSize(a);
    }

    void receiveLocalizations(const EngineResult& e) {
        if (int(e.size()) >= min_count) {
            output::Filter::receiveLocalizations(e);
        }
    }

  public:
    TraceCountFilter(const TraceCountConfig& config,
                     std::auto_ptr<output::Output> output);
};

TraceCountFilter::TraceCountFilter(
    const TraceCountConfig& c,
    std::auto_ptr<output::Output> output
)
: Filter(output),
  min_count(c.min_count()) {}

}

std::auto_ptr< output::OutputSource > make_trace_count_source() {
    return std::auto_ptr< output::OutputSource >( new FilterBuilder< TraceCountConfig, TraceCountFilter >() );
}


}
}
