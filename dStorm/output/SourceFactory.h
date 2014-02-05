#ifndef DSTORM_TRANSMISSIONSOURCEFACTORY_H
#define DSTORM_TRANSMISSIONSOURCEFACTORY_H

#include <simparm/Object.h>
#include <set>
#include "dStorm/output/Capabilities.h"
#include <boost/utility.hpp>
#include <boost/signals2/signal.hpp>
#include <boost/signals2/slot.hpp>

namespace dStorm {
namespace output {

class OutputSource;

/** An SourceFactory is an object that can configurably
 *  produce OutputSource objects. It is used by sources for
 *  filter sources to produce their outputs.
 *  */
class SourceFactory 
{
  public:
    typedef boost::signals2::signal<void (void)>::slot_type Callback;
    virtual ~SourceFactory() {}

    virtual SourceFactory* clone() const = 0;
    virtual void attach_ui( simparm::NodeHandle at ) = 0;
    virtual void set_source_capabilities(Capabilities) = 0;
    virtual std::auto_ptr<OutputSource> make_output_source() = 0;
    virtual void notify_when_output_source_is_available( const Callback& ) = 0;
};

typedef SourceFactory TransmissionSourceFactory;

}
}

#endif
