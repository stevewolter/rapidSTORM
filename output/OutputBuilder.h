#ifndef DSTORM_ENGINE_OUTPUTBUILDER_H
#define DSTORM_ENGINE_OUTPUTBUILDER_H

#include "output/OutputSource.h"
#include <simparm/Entry.h>
#include <simparm/TreeEntry.h>

namespace dStorm {
namespace output {

template <typename Config, typename Output>
class OutputBuilder
: public OutputSource
{
protected:
    Config config;
    simparm::TreeObject name_object;
    simparm::Object choice_object;
public:
    OutputBuilder();
    OutputBuilder* clone() const
        { return new OutputBuilder(*this); }
    ~OutputBuilder() {}

    void set_source_capabilities( Capabilities cap ) 
    {
        choice_object.set_visibility( config.can_work_with( cap ) );
    }

    std::auto_ptr<output::Output> make_output() 
    {
        return std::auto_ptr<output::Output>( new Output(config) );
    }

    std::string getName() const { return Config::get_name(); }
    std::string getDesc() const { return Config::get_description(); }
    void attach_full_ui( simparm::NodeHandle at ) { 
        simparm::NodeHandle r = name_object.attach_ui( at );
        config.attach_ui( r ); 
        OutputSource::attach_destruction_trigger( r );
    }
    void attach_ui( simparm::NodeHandle at ) { 
        choice_object.attach_ui( at ); 
    }
    void hide_in_tree() { name_object.show_in_tree = false; }
};

}
}

#include "output/OutputBuilder_impl.h"

#endif
