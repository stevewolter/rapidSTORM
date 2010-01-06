#ifndef DSTORM_TRACE_REDUCER
#define DSTORM_TRACE_REDUCER

#include "Trace.h"
#include <simparm/Object.hh>
#include <Eigen/Core>

namespace dStorm {
namespace output {
/** A TraceReducer class can compute the reduced position of a trace.
 *  That is, the localization that can replace the entire trace. */
class TraceReducer {
  public:
    class Config;

    virtual void reduce_trace_to_localization 
        (const Trace& from, Localization *to, 
         const Localization::Position& shift_correction) = 0;
};

/** A config object capable of configuring and making
 *  trace reducer objects. */
class TraceReducer::Config : public simparm::Object {
  protected:
    class Implementation;
    std::auto_ptr<Implementation> implementation;
  public:
    Config();
    Config(const Config&);
    ~Config();
    Config& operator=(const Config&);

    virtual Config* clone() const { return new Config(*this); }
    std::auto_ptr<TraceReducer> make_trace_reducer() const;
};

}
}

#endif
