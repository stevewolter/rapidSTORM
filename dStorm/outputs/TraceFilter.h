#ifndef DSTORM_TRACE_FILTER
#define DSTORM_TRACE_FILTER

#include "../output/FilterBuilder.h"
#include <simparm/Entry.hh>
#include "../Engine_decl.h"
#include <vector>

namespace dStorm {
namespace outputs {

class TraceCountConfig 
{
  public:
    simparm::Entry<unsigned long> min_count;
    simparm::Entry<bool> disassemble;
    simparm::Entry<bool> selectSpecific;
    simparm::Entry<unsigned long> whichSpecific;

  private:
    void set_which_viewability() { whichSpecific.viewable = selectSpecific(); }
    simparm::BaseAttribute::ConnectionStore listening;

  public:
    TraceCountConfig();

    void attach_ui( simparm::Node& at );
    static std::string get_name() { return "TraceFilter"; }
    static std::string get_description() { return "Trace filter"; }
    static simparm::Object::UserLevel get_user_level() { return simparm::Object::Intermediate; }

    bool determine_output_capabilities( output::Capabilities& cap ) {
        if ( ! cap.test( output::Capabilities::ClustersWithSources ) )
            return false;
        return true;
    }
};

std::auto_ptr< output::Output > make_trace_count_filter( const TraceCountConfig&, std::auto_ptr< output::Output > );
std::auto_ptr< output::OutputSource > make_trace_count_source();

}
}

#endif
