#include "Config_decl.h"
#include "Status_decl.h"
#include "Backend_decl.h"
#include <memory>

namespace dStorm {
namespace viewer {

template <typename Hueing> class TerminalBackend;

std::auto_ptr<Backend>
select_terminal_backend( Config& config, Status& status );

}
}
