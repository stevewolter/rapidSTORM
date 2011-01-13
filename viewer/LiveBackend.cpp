#include "LiveBackend_impl.h"
#include "colour_schemes/impl.h"
#include "Status_decl.h"
#include "Status.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) \
    template class LiveBackend< Hueing >;
#include "colour_schemes/instantiate.h"

}
}
