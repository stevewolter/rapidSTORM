#ifndef DSTORM_ENGINE_SPOTFITTERBUILDER_H
#define DSTORM_ENGINE_SPOTFITTERBUILDER_H

#include "SpotFitterFactory.h"

namespace dStorm {
namespace engine {
namespace spot_fitter {

template <typename BaseClass>
class Builder
    : public BaseClass::Config, public Factory
{
public:
    virtual Builder<BaseClass>* clone() const 
        { return new Builder<BaseClass>(*this); }
    virtual std::auto_ptr<Implementation> make (const JobInfo& info) 
        { return std::auto_ptr<Implementation>(new BaseClass( *this, info )); }
    void set_traits( output::Traits& t, const JobInfo& e )
        { BaseClass::Config::set_traits(t, e); }
    void set_requirements( input::Traits<engine::ImageStack>& t ) 
        { BaseClass::Config::set_requirements(t); }
    void register_trait_changing_nodes( simparm::Listener& ) {}
    virtual void attach_ui( simparm::Node& to ) { BaseClass::Config::attach_ui(to); }
    virtual void detach_ui( simparm::Node& to ) { BaseClass::Config::detach_ui(to); }
    virtual std::string getName() const { return BaseClass::Config::getName(); }
};

}
}
}

#endif
