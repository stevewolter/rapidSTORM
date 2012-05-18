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
    simparm::BoolEntry failSilently;
    simparm::Object name_object;
public:
    OutputBuilder(bool failSilently = false);
    OutputBuilder* clone() const
        { return new OutputBuilder(*this); }
    ~OutputBuilder() {}

    void set_source_capabilities( Capabilities cap ) 
    {
        name_object.viewable = config.can_work_with( cap );
    }

    std::auto_ptr<output::Output> make_output() 
    {
        try {
            return std::auto_ptr<output::Output>( new Output(config) );
        } catch (...) {
            if ( !failSilently() ) 
                throw;
            else
                return std::auto_ptr<output::Output>( NULL );
        }
    }

    std::string getName() const { return Config::get_name(); }
    std::string getDesc() const { return Config::get_description(); }
    void attach_full_ui( simparm::Node& at ) { 
        config.attach_ui( name_object ); 
        OutputSource::attach_source_ui( name_object );
        failSilently.attach_ui( name_object );
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
