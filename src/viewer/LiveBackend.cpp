#include "LiveBackend_impl.h"
#include "ColourDisplay_impl.h"

namespace dStorm {
namespace viewer {

template <int Index>
static Backend* select_live_backend_from(
    const Config& config
) {
    int index = ( config.colourScheme.value() )();

    if ( index == Index ) {
        return new LiveBackend<Index>( config );
    } else if ( index >= ColourSchemes::FirstColourModel
                && index <= Index ) {
        return select_live_backend_from< 
            (Index > ColourSchemes::FirstColourModel) ? Index-1 : Index >
                ( config );
    } else {
        throw std::logic_error("Invalid colour scheme.");
    }
}

std::auto_ptr<Backend>
select_live_backend( const Config& config )
{
    return std::auto_ptr<Backend>(
        select_live_backend_from<ColourSchemes::LastColourModel>(config) );
}

}
}
