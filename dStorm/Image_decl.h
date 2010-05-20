#include <dStorm/BaseImage_decl.h>
#include <dStorm/input/Traits.h>

namespace dStorm {
    template <typename PixelType, int Dimensions>
    class Image;

    namespace input {
        template <typename PixelType, int Dimensions>
        class Traits< dStorm::Image<PixelType,Dimensions> >;
    }
}
