#ifndef DSTORM_PROGRESSMETER_H
#define DSTORM_PROGRESSMETER_H

#include <dStorm/output/Output.h>
#include <dStorm/output/OutputBuilder.h>
#include <simparm/ProgressEntry.hh>

namespace dStorm {
namespace output {

/** This class updates a ProgressEntry with the progress status for the
 *  crankshaft it is added to. */
class ProgressMeter : public OutputObject
{
  private:
    ost::Mutex mutex;
    simparm::ProgressEntry progress;
    frame_count max;
    simparm::optional<frame_count> length;

  protected:
    AdditionalData announceStormSize(const Announcement &a) { 
        ost::MutexLock lock(mutex);
        length = a.traits.total_frame_count;
        max = frame_count::from_value(0);
        if ( ! progress.isActive() ) progress.makeASCIIBar( std::cerr );
        return AdditionalData(); 
    }
    virtual Result receiveLocalizations(const EngineResult& er) 
 
    {
        ost::MutexLock lock(mutex);
        if ( er.forImage+1*camera::frame > max ) {
            if ( length.is_set() ) {
                max = er.forImage+1*camera::frame;
                float ratio = 
                    quantity<camera::time>(max) / *length;
                progress.setValue( round(ratio / 0.01)
                                   * 0.01 );
            } else {
                /* TODO */
            }
        }
        return KeepRunning;
    }

    void propagate_signal(ProgressSignal s) {
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

  public:
    class Config;
    typedef OutputBuilder<ProgressMeter> Source;

    ProgressMeter(const Config &);
    ProgressMeter(const ProgressMeter& c)
        : OutputObject(c),
          progress(c.progress), max(c.max), length(c.length)
        { push_back(progress); }
    virtual ~ProgressMeter() {}

    ProgressMeter *clone() const { return new ProgressMeter(*this); }
};

class ProgressMeter::Config : public simparm::Object 
{ 
  public:
    Config(); 
    bool can_work_with(Capabilities) { return true; }
};

}
}

#endif
