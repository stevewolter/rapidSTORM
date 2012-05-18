#ifndef DSTORM_ENGINE_OUTPUTBUILDER_H
#define DSTORM_ENGINE_OUTPUTBUILDER_H

#include "OutputSource.h"
#include <simparm/Entry.hh>

namespace dStorm {
namespace output {

template < typename Type, typename BaseSource = dStorm::output::OutputSource >
class OutputBuilder;

    template < typename Type, typename BaseSource>
    class OutputBuilder
    : public Type::Config,
      public BaseSource
    {
        simparm::BoolEntry failSilently;
        simparm::Object name_object;
      public:
        typedef typename Type::Config Config;
        typedef Type BaseType;

        OutputBuilder(bool failSilently = false);
        OutputBuilder(const OutputBuilder&);
        OutputBuilder<Type,BaseSource>* clone() const
            { return new OutputBuilder<Type,BaseSource>(*this); }
        virtual ~OutputBuilder() {}

        virtual void set_source_capabilities( Capabilities cap ) 
        {
            this->viewable = this->Type::Config::can_work_with( cap );
        }

        virtual std::auto_ptr<Output> make_output() 
 
        {
            try {
                return std::auto_ptr<Output>( new Type(*this) );
            } catch (...) {
                if ( !failSilently() ) 
                    throw;
                else
                    return std::auto_ptr<Output>( NULL );
            }
        }

        std::string getName() const { return Type::Config::getName(); }
        std::string getDesc() const { return Type::Config::getDesc(); }
        void attach_full_ui( simparm::Node& at ) { 
            simparm::NodeRef r = Type::Config::attach_ui( at ); 
            BaseSource::attach_source_ui( r );
            failSilently.attach_ui( r );
        }
        void attach_ui( simparm::Node& at ) { 
            name_object.attach_ui( at ); 
        }
    };

}
}

#include "OutputBuilder_impl.h"

#endif
