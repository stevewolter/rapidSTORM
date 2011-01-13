#include "Backend.h"
#include "LiveBackend.h"
#include "TerminalBackend.h"
#include "Config.h"
#include "colour_schemes/impl.h"

namespace dStorm {
namespace viewer {

template <typename Colorizer>
std::auto_ptr<Backend> Backend::create( const Colorizer& col, Config& config, Status& status ) {
    if ( config.showOutput() )
        return std::auto_ptr<Backend>(new LiveBackend<Colorizer>(col, config, status ));
    else
        return std::auto_ptr<Backend>(new TerminalBackend<Colorizer>(col, config ));
}

#define DISC_INSTANCE(Hueing) \
    template std::auto_ptr<Backend> Backend::create<Hueing>( const Hueing&, Config&, Status& );
#include "colour_schemes/instantiate.h"

}
}
