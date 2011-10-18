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

        std::string getDesc() const 
            { return Type::Config::getDesc(); }
    };

}
}

#include "OutputBuilder_impl.h"

#endif
