#ifndef DSTORM_ENGINE_SPOTFITTERBUILDER_H
#define DSTORM_ENGINE_SPOTFITTERBUILDER_H

#include "SpotFitterFactory.h"

namespace dStorm {
namespace engine {

template <typename BaseClass>
class SpotFitterBuilder
    : public BaseClass::Config, public SpotFitterFactory
{
    public:
    SpotFitterBuilder() 
        : SpotFitterFactory(
            static_cast<typename BaseClass::Config&>
                (*this)) {}
    SpotFitterBuilder(const SpotFitterBuilder<BaseClass>& o)
        : BaseClass::Config(o), 
            SpotFitterFactory(
            static_cast<typename BaseClass::Config&>
                (*this)) {}

    virtual SpotFitterBuilder<BaseClass>* clone() const 
        { return new SpotFitterBuilder<BaseClass>(*this); }
    virtual std::auto_ptr<SpotFitter> make
        (const ConstructionInfo& info) const
        { return std::auto_ptr<SpotFitter>(new BaseClass( *this, info )); }
};

}
}

#endif
