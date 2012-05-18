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
        simparm::Object name_object;
      public:
        typedef typename Type::Config Config;
        typedef Type BaseType;

        FilterBuilder(bool failSilently = false) ;
        FilterBuilder(const FilterBuilder&);
        FilterBuilder* clone() const; 

        virtual void set_source_capabilities( Capabilities cap ); 

        virtual std::auto_ptr<Output> make_output();

        std::string getName() const { return Type::Config::getName(); }
        std::string getDesc() const { return Type::Config::getDesc(); }
        void attach_full_ui( simparm::Node& at ) { 
            simparm::NodeRef r = Type::Config::attach_ui( at ); 
            FilterSource::attach_source_ui(r); 
        }
        void attach_ui( simparm::Node& at ) { name_object.attach_ui( at ); }
    };
}
}

#include "FilterBuilder_impl.h"

#endif
