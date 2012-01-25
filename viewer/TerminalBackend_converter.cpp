#include "TerminalBackend_converter.h"
#include "LiveBackend.h"
#include "colour_schemes/impl.h"
#include "Status_decl.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) template \
    std::auto_ptr<Backend> LiveBackend<Hueing> \
        ::change_liveness( Status& s )

#include "colour_schemes/instantiate.h"

}
}
