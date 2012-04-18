#ifndef DSTORM_ENGINE_PLANE_JOINER_H
#define DSTORM_ENGINE_PLANE_JOINER_H

#include <dStorm/engine/Image.h>
#include <dStorm/image/MetaInfo.h>
#include <dStorm/traits/Projection.h>

namespace dStorm {
namespace engine {

class PlaneFlattener {
    const dStorm::engine::InputTraits& traits;
    Image2D buffer;
    typedef traits::Projection::SubpixelImagePosition Subpixel;
    typedef Image< Subpixel, 2 > Transformed;
    std::vector<Transformed> transformed;
    std::vector<float> weights;
  public:
    PlaneFlattener( const dStorm::engine::InputTraits&, std::vector<float> weights );
    const Image2D flatten_image( const engine::ImageStack& multiplane );
};

}
}

#endif
