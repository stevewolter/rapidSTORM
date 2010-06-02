#include "debug.h"
#include "ProgressMeter.h"
#include "doc/help/context.h"

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
        progress.helpID = HELP_ProgressMeter_Progress;
        progress.setEditable(false);
        progress.setViewable(true);
        progress.setUserLevel(simparm::Object::Beginner);
        progress.setIncrement(0.02);
        push_back(progress);
    }

Output::AdditionalData
ProgressMeter::announceStormSize(const Announcement &a) { 
        ost::MutexLock lock(mutex);
        first = a.traits.first_frame;
        if ( a.traits.last_frame.is_set() )
            length = *a.traits.last_frame + a.traits.first_frame
                        + 1 * cs_units::camera::frame;
        max = frame_count::from_value(0);
        if ( ! progress.isActive() ) progress.makeASCIIBar( std::cerr );
        return AdditionalData(); 
    }

Output::Result ProgressMeter::receiveLocalizations(const EngineResult& er) 
{
    ost::MutexLock lock(mutex);
    if ( er.forImage+1*cs_units::camera::frame > max ) {
        max = er.forImage+1*cs_units::camera::frame;
        DEBUG("Progress at " << max);
        if ( length.is_set() ) {
            boost::units::quantity<cs_units::camera::time,float>
                diff = (max - first);
            DEBUG("Diff is " << diff);
            float ratio = diff / *length;
            DEBUG("Ratio is " << ratio);
            progress.setValue( round(ratio / 0.01)
                                * 0.01 );
        } else {
            /* TODO */
        }
    }
    return KeepRunning;
}

void ProgressMeter::propagate_signal(ProgressSignal s)
{
    ost::MutexLock lock(mutex);
    if ( s == Engine_is_restarted) {
        progress.setValue(0); 
        max = 0;
    } else if ( s == Engine_run_succeeded ) {
        double save_increment = progress.increment();
        progress.increment = 0;
        progress.setValue(1); 
        progress.increment = save_increment;
    }
}

}
}
