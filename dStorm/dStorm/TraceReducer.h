#ifndef DSTORM_TRACE_REDUCER
#define DSTORM_TRACE_REDUCER

#include <dStorm/Trace.h>
#include <ConfigObject.hh>

namespace dStorm {
class TraceReducer {
  public:
    virtual void reduce_trace_to_localization 
        (const Trace& from, Localization *to) throw() = 0;
};

class TraceReducerConfig : public virtual simparm::ConfigObject {
  protected:
    TraceReducerConfig() throw() : simparm::ConfigObject("", "") {}
  public:
    static std::auto_ptr<TraceReducerConfig> (*factory)() throw();

    virtual TraceReducerConfig* clone() const = 0;
    virtual std::auto_ptr<TraceReducer> make_trace_reducer() throw() = 0;
};

};

#endif
