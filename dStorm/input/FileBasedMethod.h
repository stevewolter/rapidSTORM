#ifndef DSTORM_INPUT_FILEBASEDMETHOD_H
#define DSTORM_INPUT_FILEBASEDMETHOD_H

#include "Method.h"
#include <boost/utility.hpp>

namespace dStorm {
namespace input {

template <typename Object>
class FileBasedMethod 
    : public Method<Object>,
      simparm::Listener,
      boost::noncopyable
{
  protected:
    FileBasedMethod( 
        input::Config& src,
        const std::string& name,
        const std::string& desc,
        const std::string& extension_name,
        const std::string& extension );
    FileBasedMethod(
        const FileBasedMethod&, 
        input::Config& master );

    input::Config& master;
    simparm::Attribute<std::string> extension;

    void operator()( const simparm::Event& );

  public:
    /** Clone the current set and all its settings.
        *  \param newMaster The Config object that will own the clone.
        *                   Used to update references. */
    virtual FileBasedMethod<Object>* clone(Config &master) const = 0;
    simparm::FileEntry& inputFile;
};

}
}

#endif
