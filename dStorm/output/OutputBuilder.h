#ifndef DSTORM_ENGINE_OUTPUTBUILDER_H
#define DSTORM_ENGINE_OUTPUTBUILDER_H

#include "OutputSource.h"
#include <simparm/Entry.hh>

namespace dStorm {
namespace output {

template <typename Config, typename Output>
class OutputBuilder
: public OutputSource
{
protected:
    Config config;
    simparm::Object name_object;
public:
    OutputBuilder();
    OutputBuilder* clone() const
        { return new OutputBuilder(*this); }
    ~OutputBuilder() {}

    void set_source_capabilities( Capabilities cap ) 
    {
        name_object.viewable = config.can_work_with( cap );
    }

    std::auto_ptr<output::Output> make_output() 
    {
        return std::auto_ptr<output::Output>( new Output(config) );
    }

    std::string getName() const { return Config::get_name(); }
    std::string getDesc() const { return Config::get_description(); }
    void attach_full_ui( simparm::Node& at ) { 
        simparm::NodeRef r = name_object.invisible_node( at );
        config.attach_ui( r ); 
        OutputSource::attach_source_ui( r );
        name_object.attach_ui( at ); 
    }
    void attach_ui( simparm::Node& at ) { 
        name_object.attach_ui( at ); 
    }
};

}
}

#include "OutputBuilder_impl.h"

#endif
