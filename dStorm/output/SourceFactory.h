#ifndef DSTORM_TRANSMISSIONSOURCEFACTORY_H
#define DSTORM_TRANSMISSIONSOURCEFACTORY_H

#include <simparm/Object.hh>
#include <set>
#include "Capabilities.h"
#include <boost/utility.hpp>

namespace dStorm {
namespace output {

class OutputSource;

/** An SourceFactory is an object that can configurably
 *  produce OutputSource objects. It is used by sources for
 *  filter sources to produce their outputs.
 *  */
class SourceFactory : boost::noncopyable
{
    simparm::Node& node;
  public:
    SourceFactory(simparm::Node&);
    SourceFactory(simparm::Node&, const SourceFactory&);
    virtual ~SourceFactory();

    simparm::Node& getNode() { return node; }
    operator simparm::Node&() { return node; }
    const simparm::Node& getNode() const { return node; }
    operator const simparm::Node&() const { return node; }

    virtual SourceFactory* clone() const = 0;
    virtual void set_source_capabilities(Capabilities) = 0;
    virtual std::auto_ptr<OutputSource> make_output_source() = 0;
    virtual void reset_state() = 0;
};

typedef SourceFactory TransmissionSourceFactory;

}
}

#endif
