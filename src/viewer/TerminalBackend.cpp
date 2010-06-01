#include "TerminalBackend_impl.h"
#include "ColourDisplay_impl.h"
#include "Status_decl.h"

namespace dStorm {
namespace viewer {

template <int Index>
static Backend* select_terminal_backend_from(
    const Config& config
) {
    int index = ( config.colourScheme.value() )();

    if ( index == Index ) {
        return new TerminalBackend<Index>( config );
    } else if ( index >= ColourSchemes::FirstColourModel
                && index <= Index ) {
        return select_terminal_backend_from< 
            (Index > ColourSchemes::FirstColourModel) ? Index-1 : Index >
                ( config );
    } else {
        throw std::logic_error("Invalid colour scheme.");
    }
}

std::auto_ptr<Backend>
select_terminal_backend( Config& config, Status& )
{
    return std::auto_ptr<Backend>(
        select_terminal_backend_from<ColourSchemes::LastColourModel>(config) );
}

}
}
