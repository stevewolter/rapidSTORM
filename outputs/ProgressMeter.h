#ifndef DSTORM_PROGRESSMETER_H
#define DSTORM_PROGRESSMETER_H

#include <dStorm/Output.h>
#include <dStorm/OutputBuilder.h>
#include <simparm/ProgressEntry.hh>

namespace dStorm {

/** This class updates a ProgressEntry with the progress status for the
 *  crankshaft it is added to. */
class ProgressMeter : public Output, public simparm::Object
{
  private:
    ost::Mutex mutex;
    simparm::ProgressEntry progress;
    unsigned int max;
    double length;

  protected:
    AdditionalData announceStormSize(const Announcement &a) { 
        ost::MutexLock lock(mutex);
        length = a.length; 
        max = 0;
        if ( ! progress.isActive() ) progress.makeASCIIBar( std::cerr );
        return NoData; 
    }
    virtual Result receiveLocalizations(const EngineResult& er) 
 
    {
        ost::MutexLock lock(mutex);
        if ( er.forImage+1 > max ) {
            max = er.forImage+1;
            progress.setValue( round(max / length / 0.01) * 0.01 );
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
    : Node(c), Output(c), Object(c), 
      progress(c.progress), max(c.max), length(c.length)
    { push_back(progress); }
    virtual ~ProgressMeter() {}

    ProgressMeter *clone() const { return new ProgressMeter(*this); }
};

class ProgressMeter::Config : public simparm::Object 
    { public: Config(); };

}

#endif
