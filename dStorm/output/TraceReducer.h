#ifndef DSTORM_TRACE_REDUCER
#define DSTORM_TRACE_REDUCER

#include "../Localization.h"
#include <simparm/Object.hh>
#include <Eigen/Core>
#include "../helpers/make_boost_clone.h"
#include <vector>
#include <any_iterator.hpp>

namespace dStorm {
namespace output {
/** A TraceReducer class can compute the reduced position of a trace.
 *  That is, the localization that can replace the entire trace. */
class TraceReducer {
  protected:
    typedef IteratorTypeErasure::any_iterator< const Localization, boost::single_pass_traversal_tag > Input;
    typedef IteratorTypeErasure::any_iterator< Localization, boost::incrementable_traversal_tag > Output;
  public:
    class Config;

    virtual ~TraceReducer() {}
    virtual TraceReducer* clone() const = 0;

    template <typename InputIterator, typename OutputIterator>
    void reduce_trace_to_localization( 
        InputIterator start, InputIterator end,
        OutputIterator to, const samplepos& shift_correction )
        { reduce_trace_to_localization( Input(start), Input(end), Output(to), shift_correction ); }

  private:
    virtual void reduce_trace_to_localization 
        (const Input& start, const Input& end, Output o, 
         const samplepos& shift_correction) = 0;
};

/** A config object capable of configuring and making
 *  trace reducer objects. */
class TraceReducer::Config {
  public:
    std::auto_ptr<TraceReducer> make_trace_reducer() const;
    void attach_ui( simparm::Node& ) {}
};

}
}

MAKE_BOOST_CLONE(dStorm::output::TraceReducer)

#endif
