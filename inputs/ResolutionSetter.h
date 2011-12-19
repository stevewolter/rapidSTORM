#ifndef DSTORM_INPUT_RESOLUTIONSETTER_H
#define DSTORM_INPUT_RESOLUTIONSETTER_H

#include "ResolutionSetter_decl.h"
#include <dStorm/input/AdapterSource.h>
#include <dStorm/traits/resolution_config.h>

#include <simparm/TreeCallback.hh>
#include <dStorm/units/nanolength.h>
#include <simparm/Structure.hh>
#include <dStorm/input/chain/MetaInfo.h>
#include <dStorm/input/chain/Context.h>
#include <dStorm/input/chain/Link.h>

namespace dStorm {
namespace input {
namespace resolution {

class SourceConfig : public traits::resolution::Config {};

template <typename ForwardedType>
class Source 
: public input::AdapterSource<ForwardedType>
{
    SourceConfig config;

    void modify_traits( input::Traits<ForwardedType>& t ) { 
        config.set_traits(t); 
    }
  public:
    Source(
        std::auto_ptr< input::Source<ForwardedType> > backend,
        const SourceConfig& config ) 
        : input::AdapterSource<ForwardedType>( backend ), config(config) {}
};

}
}
}

#endif
