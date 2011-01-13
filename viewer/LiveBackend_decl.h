#include "Config_decl.h"
#include "Backend_decl.h"
#include <memory>

namespace dStorm {
namespace viewer {

template <typename Hueing> class LiveBackend;

std::auto_ptr<Backend>
select_live_backend( Config& config, Status& status );

}
}
