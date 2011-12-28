#ifndef DSTORM_OUTPUT_FILTER_H
#define DSTORM_OUTPUT_FILTER_H

#include "Output.h"

namespace dStorm {
namespace output {

class Filter : public Output 
{
    std::auto_ptr<Output> fwd;
  protected:
    void destroy_suboutput();
  public:
    Filter( std::auto_ptr<Output> downstream ) : fwd(downstream) {}
    Filter( const Filter& o ) : fwd( (o.fwd.get() ? o.fwd->clone() : NULL) ) {}
    Filter* clone() const = 0;
    simparm::Node& getNode() { return fwd->getNode(); }
    const simparm::Node& getNode() const { return fwd->getNode(); }

    AdditionalData announceStormSize(const Announcement& a) 
        { return fwd->announceStormSize(a); }
    RunRequirements announce_run(const RunAnnouncement& r) 
        { return fwd->announce_run(r); }
    void store_results() { fwd->store_results(); }
    void receiveLocalizations(const EngineResult& er);
    void check_for_duplicate_filenames
        (std::set<std::string>& present_filenames)
        { fwd->check_for_duplicate_filenames(present_filenames); }

};

}
}

#endif
