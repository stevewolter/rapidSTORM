#ifndef DSTORM_OUTPUT_FILTER_H
#define DSTORM_OUTPUT_FILTER_H

#include "Output.h"

namespace dStorm {
namespace output {

class Filter : public Output 
{
    std::auto_ptr<Output> fwd;
  private:
    void store_results_(bool success) { fwd->store_results(success); }
    void attach_ui_( simparm::Node& at ) { attach_children_ui(at); }
  protected:
    void destroy_suboutput();
    void prepare_destruction_() { fwd->prepare_destruction(); }
    void attach_children_ui( simparm::Node& at ) { fwd->attach_ui( at ); }
    void store_children_results( bool success ) { fwd->store_results(success); }
  public:
    Filter( std::auto_ptr<Output> downstream ) : fwd(downstream) {}
    Filter( const Filter& o ) : fwd( (o.fwd.get() ? o.fwd->clone() : NULL) ) {}
    Filter* clone() const = 0;

    AdditionalData announceStormSize(const Announcement& a) 
        { return fwd->announceStormSize(a); }
    RunRequirements announce_run(const RunAnnouncement& r) 
        { return fwd->announce_run(r); }
    void receiveLocalizations(const EngineResult& er);
    void check_for_duplicate_filenames
        (std::set<std::string>& present_filenames)
        { fwd->check_for_duplicate_filenames(present_filenames); }

};

}
}

#endif
