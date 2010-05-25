#ifndef DSTORM_INPUT_RESOLUTIONSETTER_H
#define DSTORM_INPUT_RESOLUTIONSETTER_H

#include "Source.h"
#include "Config.h"

namespace dStorm {
namespace input {

template <typename ForwardedType>
class ResolutionSetter 
: public Source<ForwardedType>, public Filter
{
    std::auto_ptr< Source<ForwardedType> > s;
    const Config& config;

  public:
    ResolutionSetter(
        std::auto_ptr< Source<ForwardedType> > backend,
        const Config& config ) 
        : Source<ForwardedType>( backend->getNode(), backend->flags ),
          s(backend), config(config) {}

    BaseSource& upstream() { return *s; }

    typedef typename Source<ForwardedType>::iterator iterator;
    typedef typename Source<ForwardedType>::TraitsPtr TraitsPtr;

    void dispatch(BaseSource::Messages m) { s->dispatch(m); }
    iterator begin() { return s->begin(); }
    iterator end() { return s->end(); }
    TraitsPtr get_traits() {
        TraitsPtr rv = s->get_traits();
        if ( ! rv->resolution.is_set() )
            rv->resolution = config.get_resolution();
        return rv;
    }
};

}
}


#endif
