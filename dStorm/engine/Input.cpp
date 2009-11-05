#include <dStorm/input/Buffer_impl.h>
#include <dStorm/input/Slot_impl.h>
#include <dStorm/input/ImageTraits.h>
#include <dStorm/engine/Input.h>
#include <dStorm/engine/Image_impl.h>

namespace CImgBuffer {
    template class Buffer<dStorm::Image>;
    template class Slot<dStorm::Image>;
}
