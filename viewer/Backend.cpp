#include "viewer/Backend.h"
#include "viewer/LiveBackend.h"
#include "viewer/TerminalBackend.h"
#include "viewer/Config.h"
#include "viewer/Status.h"
#include "viewer/ColourScheme.h"

namespace dStorm {
namespace viewer {

std::auto_ptr<Backend> Backend::create( std::auto_ptr< ColourScheme > col, Status& status ) {
    if ( status.config.showOutput() )
        return std::auto_ptr<Backend>(new LiveBackend(col, status ));
    else
        return std::auto_ptr<Backend>(new TerminalBackend(col, status ));
}

}
}
