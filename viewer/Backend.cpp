#include "Backend.h"
#include "LiveBackend.h"
#include "TerminalBackend.h"
#include "Config.h"
#include "Status.h"
#include "ColourScheme.h"

namespace dStorm {
namespace viewer {

std::auto_ptr<Backend> Backend::create( std::auto_ptr< ColourScheme > col, Status& status ) {
    if ( status.config.showOutput() )
        return std::auto_ptr<Backend>(new LiveBackend<ColourScheme>(col, status ));
    else
        return std::auto_ptr<Backend>(new TerminalBackend(col, status ));
}

}
}
