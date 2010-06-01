#include "LiveBackend_impl.h"
#include "ColourDisplay_impl.h"
#include "Status_decl.h"
#include "Status.h"

namespace dStorm {
namespace viewer {

template <int Index>
static Backend* select_live_backend_from(
    Config& config, Status& status
) {
    int index = ( config.colourScheme.value() )();

    if ( index == Index ) {
        return new LiveBackend<Index>( config, status );
    } else if ( index >= ColourSchemes::FirstColourModel
                && index <= Index ) {
        return select_live_backend_from< 
            (Index > ColourSchemes::FirstColourModel) ? Index-1 : Index >
                ( config, status );
    } else {
        throw std::logic_error("Invalid colour scheme.");
    }
}

std::auto_ptr<Backend>
select_live_backend( Config& config, Status& status )
{
    return std::auto_ptr<Backend>(
        select_live_backend_from<ColourSchemes::LastColourModel>(config, status) );
}

}
}
