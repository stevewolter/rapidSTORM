#ifndef CIMGBUFFER_INPUTMETHOD_H
#define CIMGBUFFER_INPUTMETHOD_H

#include <data-c++/AutoList.h>
#include <memory>
#include <simparm/Set.hh>
#include <CImgBuffer/Config.h>
#include <CImgBuffer/Source.h>

namespace CImgBuffer {
    class BaseSource;
    template <typename Type> class Source;

    /** A BaseInputConfig class is the base class for all config sets
     *  that can provide a Source object. */
    class BaseInputConfig : public simparm::Object {
      protected:
        BaseInputConfig(const std::string& name, const std::string& desc)
 : simparm::Object(name, desc) {}
        virtual BaseSource *impl_makeSource() = 0;
     
      public:
        virtual bool may_be_autoloaded() const { return true; }

        /** This method is provided for compatibility with InputConfig
         *  and always fails. */
        BaseInputConfig *clone() const
            { throw std::logic_error("No clone() method defined "
                                     "for InputConfig objects."); }
        /** Clone the current set and all its settings.
         *  \param master    The Config object that will own the clone.
         *                   Used to update references. */
        virtual BaseInputConfig* clone(Config &master) const = 0;

        /** Make a Source object with the current settings. */
        std::auto_ptr< BaseSource > makeSource() 
            { return std::auto_ptr<BaseSource>(impl_makeSource()); }
    };

    template <typename T>
    class InputConfig : public BaseInputConfig {
      protected:
        InputConfig(const std::string& name, const std::string& desc)
            : BaseInputConfig(name, desc) {}
      public:
        /** Make a Source object with the current settings. */
        std::auto_ptr< Source<T> > makeSource()
            { return std::auto_ptr< Source<T> >(impl_makeSource()); }
        /** Clone the current set and all its settings.
         *  \param newMaster The Config object that will own the clone.
         *                   Used to update references. */
        virtual InputConfig<T>* clone(Config &master) const = 0;
    };
}
#endif
