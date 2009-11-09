#ifndef CIMGBUFFER_AUTOINPUTMETHOD_H
#define CIMGBUFFER_AUTOINPUTMETHOD_H

#include "Method.h"
#include <simparm/Set.hh>

namespace dStorm {
namespace input {
    /** The AutoMethod produces an source by trying to build
     *  a source from each of the input configs registered at its
     *  master config. Sources that have \c may_be_autoloaded as
     *  false will not be tried, and sources that throw errors on
     *  construction will not be considered. The first input config
     *  that has the may_be_autoloaded property and throws no error
     *  on source production will be selected and its source 
     *  returned. */
    class AutoMethod 
    : public BaseMethod
    {
      private:
        Config& master;
        bool recursive;
        AutoMethod(const AutoMethod&);

      protected:
        BaseSource* impl_makeSource()
        {
            /* Not necessary per se, but two nets hold better. */
            if (recursive) 
                throw std::logic_error("Recursive invocation");

            recursive = true;
            std::auto_ptr< BaseSource > rv;
            
            simparm::NodeChoiceEntry< BaseMethod >::iterator 
              i ( master.inputMethod.beginChoices() );
            for ( ; i != master.inputMethod.endChoices(); i++)
            {
                if ( ! i->may_be_autoloaded() ) continue;
                try {
                    rv = i->makeSource( master );
                } catch (const std::exception&) {
                    /* The exception means the source is unable to read
                     * that file type. */
                }
                if (rv.get() != NULL) { 
                    master.inputMethod.setValue( *i );
                    break;
                }
            }

            recursive = false;
            if (rv.get() == NULL) 
                throw std::logic_error(
                    "Input file type could not be recognized");
            return rv.release();
        }

      public:
        AutoMethod(Config& master) 
        : BaseMethod("Auto", "File"),
          master(master), recursive(false) 
        {
            this->register_entry(&master.inputFile);
            this->register_entry(&master.firstImage);
            this->register_entry(&master.lastImage);
        }

        bool may_be_autoloaded() const { return false; }

        AutoMethod* clone() const
            { throw std::logic_error("AutoMethod unclonable."); }
        AutoMethod* clone
            (Config &newMaster) const 
            { return (new AutoMethod(newMaster)); }
    };
}
}

#endif
