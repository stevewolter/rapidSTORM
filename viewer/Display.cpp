#include "Display_impl.h"
#include "colour_schemes/impl.h"

namespace dStorm {
namespace viewer {

#define DISC_INSTANCE(Hueing) template class Display< Hueing >

#include "colour_schemes/instantiate.h"

}
}
