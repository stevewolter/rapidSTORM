#include "Backend.h"
#include "LiveBackend.h"
#include "TerminalBackend.h"
#include "Config.h"
#include "Status.h"
#include "colour_schemes/impl.h"

namespace dStorm {
namespace viewer {

template <typename Colorizer>
std::auto_ptr<Backend> Backend::create( const Colorizer& col, Status& status ) {
    if ( status.config.showOutput() )
        return std::auto_ptr<Backend>(new LiveBackend<Colorizer>(col, status ));
    else
        return std::auto_ptr<Backend>(new TerminalBackend<Colorizer>(col, status ));
}

#define DISC_INSTANCE(Hueing) \
    template std::auto_ptr<Backend> Backend::create<Hueing>( const Hueing&, Status& );
#include "colour_schemes/instantiate.h"

}
}
