#include "TerminalBackend_impl.h"
#include "colour_schemes/impl.h"
#include "Status_decl.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) \
    template class TerminalBackend< Hueing >;
#include "colour_schemes/instantiate.h"

}
}
