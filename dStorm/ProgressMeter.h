#ifndef DSTORM_PROGRESSMETER_H
#define DSTORM_PROGRESSMETER_H

#include <dStorm/Transmission.h>
#include <dStorm/Image.h>

namespace dStorm {

/** This class updates a ConfigEntryProgress with the progress status for the
 *  crankshaft it is added to. */
class ProgressMeter : public Transmission
{
  private:
    ost::Mutex mutex;
    ConfigEntryProgress progress;
    unsigned int max;
    double length;
  protected:
    AdditionalData announceStormSize(int, int, int l) throw() { 
        ost::MutexLock lock(mutex);
        length = l; 
        max = 0;
        if ( ! progress.isActive() ) progress.makeASCIIBar( std::cerr );
        return NoData; 
    }
    virtual Result receiveLocalizations(const EngineResult& er) 
            throw() 
    {
        ost::MutexLock lock(mutex);
        if ( er.forImage+1 > max ) {
            max = er.forImage+1;
            progress.setValue(max / length);
        }
        return KeepRunning;
    }

    virtual void deleteAllResults() throw() { 
        ost::MutexLock lock(mutex);
        progress.setValue(0); 
        max = 0;
    }
    virtual void finish() throw() { 
        ost::MutexLock lock(mutex);
        progress.setValue(1); 
    }
  public:
    ProgressMeter() throw()
    : ConfigObject("ProgressMeter", "Progress status"),
      progress("Progress", "Progress on this job") 
    {
        progress.setEditable(false);
        progress.setViewable(true);
        progress.setUserLevel(ConfigEntry::Beginner);
        progress.setIncrement(0.02);
        push_back(progress);
    }
    virtual ~ProgressMeter() throw() {}

    const char *getName() throw() { return "ProgressMeter"; }
};

}

#endif
