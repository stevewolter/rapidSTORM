#ifndef DSTORM_ENGINE_CONFIGURED_SOURCE_H
#define DSTORM_ENGINE_CONFIGURED_SOURCE_H

#include "FilterSource.h"
#include <stdexcept>

namespace dStorm {
namespace output {

    template <typename Type, typename OutputType>
    class FilterBuilder 
    : public FilterSource
    {
        Type config;
        simparm::Object name_object;
      public:
        typedef Type BaseType;

        FilterBuilder() ;
        FilterBuilder(const FilterBuilder&);
        FilterBuilder* clone() const; 

        virtual void set_source_capabilities( Capabilities cap ); 

        virtual std::auto_ptr<Output> make_output();

        std::string getName() const { return Type::get_name(); }
        std::string getDesc() const { return Type::get_description(); }
        void attach_full_ui( simparm::Node& at ) { 
            simparm::NodeRef r = name_object.invisible_node(at);
            config.attach_ui( r );
            FilterSource::attach_source_ui( r ); 
            name_object.attach_ui( at ); 
        }
        void attach_ui( simparm::Node& at ) { name_object.attach_ui( at ); }
    };
}
}

#include "FilterBuilder_impl.h"

#endif
