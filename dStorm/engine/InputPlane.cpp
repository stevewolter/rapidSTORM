#include "dStorm/engine/InputPlane.h"
#include <dStorm/traits/Projection.h>
#include <dStorm/traits/ProjectionFactory.h>

namespace dStorm {
namespace engine {

InputPlane::~InputPlane() {}

void InputPlane::create_projection() {
    projection_ = optics.projection_factory()->get_projection( image );
}

}
}
