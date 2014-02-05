#include "engine/InputPlane.h"
#include "traits/Projection.h"
#include "traits/ProjectionFactory.h"

namespace dStorm {
namespace engine {

InputPlane::~InputPlane() {}

void InputPlane::create_projection() {
    projection_ = optics.projection_factory()->get_projection( image );
}

}
}
