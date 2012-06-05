#include "debug.h"
#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <simparm/ProgressEntry.h>

#include "ProgressMeter.h"

namespace dStorm {
namespace output {

/** This class updates a ProgressEntry with the progress status for the
 *  crankshaft it is added to. */
class ProgressMeter : public Output
{
  private:
    simparm::ProgressEntry progress;
    simparm::NodeHandle current_ui;
    frame_count max;
    frame_count first;
    boost::optional<frame_count> length;
    void attach_ui_( simparm::NodeHandle at ) {
        current_ui = progress.attach_ui(at);
    }

  protected:
    AdditionalData announceStormSize(const Announcement &a);
    void receiveLocalizations(const EngineResult& er);
    void store_results_( bool success );

  public:
    class Config;

    ProgressMeter(const Config &);
    virtual ~ProgressMeter() {}
    RunRequirements announce_run(const RunAnnouncement&) {
        progress.setValue(0); 
        max = 0;
        return RunRequirements();
    }

};

class ProgressMeter::Config 
{ 
  public:
    bool can_work_with(Capabilities) { return true; }
    void attach_ui( simparm::NodeHandle ) {}
    static std::string get_name() { return "Progress"; }
    static std::string get_description() { return "Display progress"; }
    static simparm::UserLevel get_user_level() { return simparm::Beginner; }
};

ProgressMeter::ProgressMeter(const Config &)
    : progress("Progress", "Progress on this job") 
    {
        progress.setHelpID( "#ProgressMeter_Progress" );
        progress.setEditable(false);
        progress.increment = (0.02);
    }

Output::AdditionalData
ProgressMeter::announceStormSize(const Announcement &a) { 
        first = *a.image_number().range().first;
        if ( a.image_number().range().second.is_initialized() )
            length = *a.image_number().range().second + first
                        + 1 * camera::frame;
        else
            progress.indeterminate = true;
        max = frame_count::from_value(0);
        return AdditionalData(); 
    }

void ProgressMeter::receiveLocalizations(const EngineResult& er) 
{
    if ( er.forImage+1*camera::frame > max ) {
        max = er.forImage+1*camera::frame;
        DEBUG("Progress at " << max);
        if ( length.is_initialized() ) {
            boost::units::quantity<camera::time,float>
                diff = (max - first);
            DEBUG("Diff is " << diff);
            float ratio = diff / *length;
            DEBUG("Ratio is " << ratio << " at progress " << progress());
            progress.setValue( std::min( round(ratio / 0.01), 99.0 ) * 0.01 );
        } else {
            progress.setValue( (er.forImage / (10*camera::frame)) % 100 / 100.0 );
        }
    }
}

void ProgressMeter::store_results_( bool success )
{
    double save_increment = progress.increment();
    progress.increment = 0;
    progress.setValue(1); 
    progress.increment = save_increment;
}

std::auto_ptr< output::OutputSource > make_progress_meter_source() {
    return std::auto_ptr< output::OutputSource >( new OutputBuilder< ProgressMeter::Config, ProgressMeter >() );
}


}
}
