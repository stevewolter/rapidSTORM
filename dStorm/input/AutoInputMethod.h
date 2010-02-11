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
    : public BaseMethod, simparm::Node::Callback
    {
      private:
        Config& master;
        bool recursive;
        const simparm::FileEntry& inputFile;
        AutoMethod(const AutoMethod&);

        void operator()( const simparm::Event& ) {
            BaseMethod::output_file_basename =
                inputFile.without_extension();
        }

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
                if ( ! i->uses_input_file() ) continue;
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
          simparm::Node::Callback( simparm::Event::ValueChanged ),
          master(master), recursive(false) ,
          inputFile( master.inputFile )
        {
            this->push_back(master.inputFile);
            this->push_back(master.firstImage);
            this->push_back(master.lastImage);

            receive_changes_from( master.inputFile.value );
        }

        AutoMethod(const AutoMethod& am, Config& master) 
        : BaseMethod(am),
          simparm::Node::Callback( simparm::Event::ValueChanged ),
          master(master), recursive(false) ,
          inputFile( master.inputFile )
        {
            this->push_back(master.inputFile);
            this->push_back(master.firstImage);
            this->push_back(master.lastImage);

            receive_changes_from( master.inputFile.value );
        }

        bool uses_input_file() const { return true; }

        AutoMethod* clone() const
            { throw std::logic_error("AutoMethod unclonable."); }
        AutoMethod* clone
            (Config &newMaster) const 
            { return (new AutoMethod(*this, newMaster)); }
    };
}
}

#endif
