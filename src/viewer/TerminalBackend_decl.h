#include "Config_decl.h"
#include "Backend_decl.h"
#include <memory>

namespace dStorm {
namespace viewer {

template <int Hueing> class TerminalBackend;

std::auto_ptr<Backend>
select_terminal_backend( const Config& config );

}
}
