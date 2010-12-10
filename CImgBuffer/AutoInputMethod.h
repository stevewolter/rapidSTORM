#ifndef CIMGBUFFER_AUTOINPUTMETHOD_H
#define CIMGBUFFER_AUTOINPUTMETHOD_H

#include "InputMethod.h"

namespace CImgBuffer {
    template <typename T>
    class AutoInputConfig : public virtual InputConfig<T> {
      private:
        Config<T>& master;
        bool recursive;
      public:
        AutoInputConfig(Config<T>& master) throw() 
        : ConfigObject("Auto", "File"),
          master(master), recursive(false) 
        {
            register_entry(&master.inputFile);
            register_entry(&master.firstImage);
            register_entry(&master.lastImage);
        }

        std::auto_ptr< Source<T> > makeSource() 
            throw(std::exception)
        {
            if (recursive) 
                throw std::logic_error("Recursive invocation");

            recursive = true;
            std::auto_ptr< Source<T> > rv;
            
            typename 
              ConfigEntryTypedChoice< InputConfig<T> >::const_iterator 
              i ( master.inputMethod.beginChoices() );
            for ( ; i != master.inputMethod.endChoices(); i++)
            {
                if ( ! i->may_be_autoloaded() ) continue;
                try {
                    rv = const_cast< InputConfig<T>& >(*i).makeSource();
                } catch (const std::exception& c) {
                    std::cerr << c.what() << "\n";
                }
                if (rv.get() != NULL) { 
                    master.inputMethod.setValue( i->getName() );
                    break;
                }
            }

            recursive = false;
            if (rv.get() == NULL) 
                throw std::logic_error(
                    "File type could not be recognized");
            return rv;
        }

        bool may_be_autoloaded() const throw() { return false; }

        std::auto_ptr< InputConfig<T> > clone
            (Config<T> &newMaster) const throw() 
            { return std::auto_ptr< InputConfig<T> >
                (new AutoInputConfig<T>(newMaster)); }

        void registerEntries(ConfigSet &) throw() {}
    };

    template <typename T>
    class AutoInputMethod : public InputMethod<T> {
      public:
        std::auto_ptr< InputConfig<T> > makeConfig
            (Config<T> &m) throw(std::exception)
        { 
            return std::auto_ptr< InputConfig<T> >
                (new AutoInputConfig<T>(m));
        }
    };
    
};

#endif
