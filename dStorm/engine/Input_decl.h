#include <dStorm/engine/Image_decl.h>
#include <dStorm/input/ImageTraits_decl.h>
#include <dStorm/input/Buffer_decl.h>

namespace dStorm {
namespace engine {
    /** Input buffer for engine. */
    typedef dStorm::input::Buffer<Image> Input;
    /** Traits for input image for STORM engine. */
    typedef dStorm::input::Traits<Image> InputTraits;
    typedef InputTraits Traits;
}
}
