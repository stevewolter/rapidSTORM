#ifndef DSTORM_INPUT_INPUTMETHOD_H
#define DSTORM_INPUT_INPUTMETHOD_H

#include <dStorm/data-c++/AutoList.h>
#include <memory>
#include <simparm/Set.hh>

#include "Config.h"
#include "Source.h"

namespace dStorm {
namespace input {

/** A BaseMethod class is the base class for all config sets
    *  that can provide a Source object. */
class BaseMethod : public simparm::Object {
  protected:
    BaseMethod(const std::string& name, const std::string& desc);
    virtual BaseSource *impl_makeSource() = 0;
    
  public:
    virtual bool uses_input_file() const { return true; }
    simparm::Attribute<std::string> output_file_basename;

    /** This method is provided for compatibility with simparm::Object
        *  and always fails. */
    BaseMethod *clone() const;
    /** Clone the current set and all its settings.
        *  \param master    The Config object that will own the clone.
        *                   Used to update references. */
    virtual BaseMethod* clone(Config &master) const = 0;

    /** Make a Source object with the current settings. */
    std::auto_ptr< BaseSource > makeSource(const Config &master);
};

template <typename T>
class Method : public BaseMethod {
    protected:
    Method(const std::string& name, const std::string& desc)
        : BaseMethod(name, desc) {}
    public:
    /** Make a Source object with the current settings. */
    std::auto_ptr< Source<T> > makeSource(const Config &master) { 
        std::auto_ptr< Source<T> > src(impl_makeSource()); 
        src->apply_global_settings( master );
        return src;
    }
    /** Clone the current set and all its settings.
        *  \param newMaster The Config object that will own the clone.
        *                   Used to update references. */
    virtual Method<T>* clone(Config &master) const = 0;
};

}
}
#endif
