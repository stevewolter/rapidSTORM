#ifndef DSTORM_ENGINE_OUTPUTBUILDER_H
#define DSTORM_ENGINE_OUTPUTBUILDER_H

#include <dStorm/OutputSource.h>
#include <simparm/NumericEntry.hh>

namespace dStorm {

    template < typename Type >
    class OutputBuilder
    : public Type::Config,
      public OutputSource
    {
        simparm::BoolEntry failSilently;
      public:
        OutputBuilder(bool failSilently = false) 
        : failSilently("FailSilently", 
                       "Allow transmission to fail silently", failSilently)
        { this->failSilently.userLevel = simparm::Entry::Debug;
          push_back( this->failSilently ); }
        OutputBuilder(const OutputBuilder& o) 
 
        : Node(o), Type::Config(o),
          OutputSource(o),
          failSilently(o.failSilently)
        { push_back( failSilently ); }
        OutputBuilder<Type>* clone() const
            { return new OutputBuilder<Type>(*this); }
        virtual ~OutputBuilder() { simparm::Node::removeFromAllParents(); }

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

#endif
