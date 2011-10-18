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
    Builder() 
        : Factory(
            static_cast<typename BaseClass::Config&>
                (*this)) {}
    Builder(const Builder<BaseClass>& o)
        : BaseClass::Config(o), 
            Factory(
            static_cast<typename BaseClass::Config&>
                (*this)) {}

    virtual Builder<BaseClass>* clone() const 
        { return new Builder<BaseClass>(*this); }
    virtual std::auto_ptr<Implementation> make (const JobInfo& info) 
        { return std::auto_ptr<Implementation>(new BaseClass( *this, info )); }
    void set_traits( output::Traits& t, const JobInfo& e )
        { BaseClass::Config::set_traits(t, e); }
    void set_requirements( input::Traits<engine::Image>& t ) 
        { BaseClass::Config::set_requirements(t); }
    void register_trait_changing_nodes( simparm::Listener& ) {}
};

}
}
}

#endif
