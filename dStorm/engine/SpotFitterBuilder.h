#ifndef DSTORM_ENGINE_SPOTFITTERBUILDER_H
#define DSTORM_ENGINE_SPOTFITTERBUILDER_H

#include "SpotFitterFactory.h"

namespace dStorm {
namespace engine {
namespace spot_fitter {

template <typename Config, typename Fitter>
class Builder : public Factory
{
    simparm::Object name_object;
    Config config;
public:
    virtual Builder* clone() const { return new Builder(*this); }
    virtual std::auto_ptr<Implementation> make (const JobInfo& info) 
        { return std::auto_ptr<Implementation>(new Fitter( *this, info )); }
    void set_traits( output::Traits& t, const JobInfo& e )
        { config.set_traits(t, e); }
    void set_requirements( input::Traits<engine::ImageStack>& t ) 
        { config.set_requirements(t); }
    void register_trait_changing_nodes( simparm::Listener& ) {}
    void attach_ui( simparm::NodeHandle to ) { 
        config.attach_ui(name_object); 
        name_object.attach_ui( to );
    }
    void detach_ui( simparm::NodeHandle to ) { config.detach_ui(to); }
    std::string getName() const { return config.getName(); }
};

}
}
}

#endif
