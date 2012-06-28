#include "TerminalBackend_converter.h"
#include "LiveBackend.h"
#include "ColourScheme.h"
#include "Status_decl.h"

namespace dStorm {
namespace viewer {

template std::auto_ptr<Backend> LiveBackend<ColourScheme>::change_liveness( Status& s );

}
}
