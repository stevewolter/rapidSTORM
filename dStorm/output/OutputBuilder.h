#ifndef DSTORM_ENGINE_OUTPUTBUILDER_H
#define DSTORM_ENGINE_OUTPUTBUILDER_H

#include "OutputSource.h"
#include <simparm/NumericEntry.hh>

namespace dStorm {
namespace output {

    template < typename Type >
    class OutputBuilder
    : public Type::Config,
      public OutputSource
    {
        simparm::BoolEntry failSilently;
      public:

        OutputBuilder(bool failSilently = false);
        OutputBuilder(const OutputBuilder&);
        OutputBuilder<Type>* clone() const
            { return new OutputBuilder<Type>(*this); }
        virtual ~OutputBuilder() {}

        virtual void set_source_capabilities( Capabilities cap ) 
        {
            this->viewable = this->Type::Config::can_work_with( cap );
        }

        virtual std::auto_ptr<Output> make_output() 
 
        {
            typename Type::Config& config =
                static_cast<typename Type::Config&>(*this);
            try {
                return std::auto_ptr<Output>( new Type(config) );
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
