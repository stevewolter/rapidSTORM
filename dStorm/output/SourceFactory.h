#ifndef DSTORM_TRANSMISSIONSOURCEFACTORY_H
#define DSTORM_TRANSMISSIONSOURCEFACTORY_H

#include <simparm/Object.hh>
#include <set>

namespace dStorm {
namespace output {

class OutputSource;

/** An SourceFactory is an object that can configurably
 *  produce OutputSource objects. It is used by sources for
 *  filter sources to produce their outputs.
 *  */
class SourceFactory
: public virtual simparm::Node
{
  public:
    SourceFactory();
    SourceFactory(const SourceFactory&);
    SourceFactory& operator=(const SourceFactory&);
    virtual ~SourceFactory();

    virtual SourceFactory* clone() const = 0;
    virtual std::auto_ptr<OutputSource> make_output_source() = 0;
    virtual void reset_state() = 0;

    typedef int BasenameResult;
    virtual BasenameResult set_output_file_basename(
        const std::string& new_basename, std::set<std::string>& avoid) = 0;
};

typedef SourceFactory TransmissionSourceFactory;

}
}

#endif
