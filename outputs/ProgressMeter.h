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
    simparm::ProgressEntry progress;
    frame_count max;
    frame_count first;
    boost::optional<frame_count> length;

  protected:
    AdditionalData announceStormSize(const Announcement &a);
    void receiveLocalizations(const EngineResult& er);
    void store_results_( bool success );

  public:
    class Config;
    typedef OutputBuilder<ProgressMeter> Source;

    ProgressMeter(const Config &);
    ProgressMeter(const ProgressMeter& c)
        : OutputObject(c),
          progress(c.progress), max(c.max), length(c.length)
        { push_back(progress); }
    virtual ~ProgressMeter() {}
    RunRequirements announce_run(const RunAnnouncement&) {
        progress.setValue(0); 
        max = 0;
        return RunRequirements();
    }

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
