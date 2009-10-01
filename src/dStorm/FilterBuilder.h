#ifndef DSTORM_ENGINE_CONFIGURED_SOURCE_H
#define DSTORM_ENGINE_CONFIGURED_SOURCE_H

#include <dStorm/FilterSource.h>
#include <stdexcept>

namespace dStorm {

    struct Source_Is_Transparent {
        std::auto_ptr<Output> output;
    };

    template <typename Type>
    class FilterBuilder 
    : public Type::Config,
      public FilterSource
    {
        bool failSilently;
      public:
        FilterBuilder(bool failSilently = false) 
        : FilterSource(), failSilently(failSilently) {}
        FilterBuilder(const FilterBuilder& o) 
        : simparm::Node(o), Type::Config(o),
          FilterSource(o), failSilently(failSilently) {}
        FilterBuilder* clone() const { return new FilterBuilder(*this); }
        virtual ~FilterBuilder() { simparm::Node::removeFromAllParents(); }

        virtual std::auto_ptr<Output> make_output() 
 
        {
            typename Type::Config& config =
                static_cast<typename Type::Config&>(*this);
            try {
                return std::auto_ptr<Output>( new Type( 
                    config, FilterSource::make_output() ) );
            } catch ( Source_Is_Transparent& transparent ) {
                return transparent.output;
            } catch (...) {
                if ( !failSilently ) 
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
