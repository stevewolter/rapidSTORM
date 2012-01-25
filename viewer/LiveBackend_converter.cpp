#include "LiveBackend_converter.h"
#include "TerminalBackend.h"
#include "colour_schemes/impl.h"
#include "Status_decl.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) template \
    std::auto_ptr<Backend> TerminalBackend<Hueing> \
        ::change_liveness( Status& s )

#include "colour_schemes/instantiate.h"

}
}
