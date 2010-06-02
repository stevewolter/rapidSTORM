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
    frame_count first;
    simparm::optional<frame_count> length;

  protected:
    AdditionalData announceStormSize(const Announcement &a);
    Result receiveLocalizations(const EngineResult& er);

    void propagate_signal(ProgressSignal s);

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
