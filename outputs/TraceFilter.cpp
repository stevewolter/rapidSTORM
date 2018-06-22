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
    binning::FieldChoice field;

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
            "Minimum number of emissions per trace", 0),
  field("Field", "Filtered field", binning::IsUnscaled, "") {}

void TraceCountConfig::attach_ui( simparm::NodeHandle at )
{
    min_count.attach_ui(at);
    field.attach_ui(at);
}

class TraceCountFilter : public output::Filter
{
  private:
    const int min_count;
    std::unique_ptr<binning::Unscaled> binner;

    LocalizedImage current_run;
    int current_group;

    AdditionalData announceStormSize(const Announcement &a) {
        binner->announce(a);
        return output::Filter::announceStormSize(a);
    }

    void receiveLocalizations(const EngineResult& e) {
        for (const Localization& l : e) {
            boost::optional<float> value = binner->bin_point(l);
            long group = std::lround(*value);

            if (!current_run.empty() && current_group != group) {
                if (group < current_group) {
                    throw std::runtime_error("Input for trace filter is not "
                        "sorted on the filtered field");
                }

                if (int(current_run.size()) >= min_count) {
                    output::Filter::receiveLocalizations(current_run);
                }
                current_run.clear();
            }

            current_group = group;
            current_run.push_back(l);
        }
    }

    void store_results( bool job_successful ) {
        if (!current_run.empty()) {
            if (int(current_run.size()) >= min_count) {
                receiveLocalizations(current_run);
            }
            current_run.clear();
        }
        output::Filter::store_results(job_successful);
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
  min_count(c.min_count()),
  binner(c.field().make_unscaled_binner()) {}

}

std::auto_ptr< output::OutputSource > make_trace_count_source() {
    return std::auto_ptr< output::OutputSource >( new FilterBuilder< TraceCountConfig, TraceCountFilter >() );
}


}
}
