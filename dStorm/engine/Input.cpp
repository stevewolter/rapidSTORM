#include <dStorm/input/Buffer_impl.h>
#include <dStorm/input/Slot_impl.h>
#include <dStorm/input/ImageTraits.h>
#include <dStorm/engine/Input.h>
#include <dStorm/engine/Image_impl.h>

namespace dStorm {
namespace input {
    template class Buffer<dStorm::engine::Image>;
    template class Slot<dStorm::engine::Image>;
}
}
