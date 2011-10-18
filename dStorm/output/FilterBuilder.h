#ifndef DSTORM_ENGINE_CONFIGURED_SOURCE_H
#define DSTORM_ENGINE_CONFIGURED_SOURCE_H

#include "FilterSource.h"
#include <stdexcept>

namespace dStorm {
namespace output {

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
        typedef typename Type::Config Config;
        typedef Type BaseType;

        FilterBuilder(bool failSilently = false) ;
        FilterBuilder(const FilterBuilder&);
        FilterBuilder* clone() const; 

        virtual void set_source_capabilities( Capabilities cap ); 

        virtual std::auto_ptr<Output> make_output();

        std::string getDesc() const 
            { return Type::Config::getDesc(); }
    };
}
}

#include "FilterBuilder_impl.h"

#endif
