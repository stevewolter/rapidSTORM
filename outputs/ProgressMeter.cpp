#include "debug.h"
#include "ProgressMeter.h"

namespace dStorm {
namespace output {

ProgressMeter::Config::Config() 
: simparm::Object( "Progress", "Display progress" ) 
{
    userLevel = Intermediate;
}

ProgressMeter::ProgressMeter(const Config &)
    : OutputObject("ProgressMeter", "Progress status"),
      progress("Progress", "Progress on this job") 
    {
        progress.helpID = "#ProgressMeter_Progress";
        progress.setEditable(false);
        progress.setViewable(true);
        progress.setUserLevel(simparm::Object::Beginner);
        progress.setIncrement(0.02);
        push_back(progress);
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
        if ( ! progress.isActive() ) progress.makeASCIIBar( std::cerr );
        return AdditionalData(); 
    }

Output::Result ProgressMeter::receiveLocalizations(const EngineResult& er) 
{
    if ( er.forImage+1*camera::frame > max ) {
        max = er.forImage+1*camera::frame;
        DEBUG("Progress at " << max);
        if ( length.is_initialized() ) {
            boost::units::quantity<camera::time,float>
                diff = (max - first);
            DEBUG("Diff is " << diff);
            float ratio = diff / *length;
            DEBUG("Ratio is " << ratio);
            progress.setValue( std::min( round(ratio / 0.01), 99.0 ) * 0.01 );
        } else {
            progress.setValue(0.5);
        }
    }
    return KeepRunning;
}

void ProgressMeter::propagate_signal(ProgressSignal s)
{
    if ( s == Engine_is_restarted) {
        progress.setValue(0); 
        max = 0;
    } else if ( s == Engine_run_succeeded || s == Engine_run_failed ) {
        double save_increment = progress.increment();
        progress.increment = 0;
        progress.setValue(1); 
        progress.increment = save_increment;
    }
}

}
}
