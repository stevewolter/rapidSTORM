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
        simparm::TreeObject name_object;
        simparm::Object choice_object;
      public:
        typedef Type BaseType;

        FilterBuilder() ;
        FilterBuilder(const FilterBuilder&);
        FilterBuilder* clone() const; 

        virtual void set_source_capabilities( Capabilities cap ); 

        virtual std::auto_ptr<Output> make_output();

        std::string getName() const { return Type::get_name(); }
        std::string getDesc() const { return Type::get_description(); }
        void attach_full_ui( simparm::NodeHandle at ) { 
            simparm::NodeHandle r = name_object.attach_ui(at);
            config.attach_ui( r );
            OutputSource::attach_destruction_trigger( r );
            FilterSource::attach_children_ui( r ); 
        }
        void attach_ui( simparm::NodeHandle at ) { choice_object.attach_ui( at ); }
        void hide_in_tree() { name_object.show_in_tree = false; }
    };
}
}

#include "FilterBuilder_impl.h"

#endif
