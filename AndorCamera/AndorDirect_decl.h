#ifndef ANDORCAMERA_ANDORDIRECT_DECL_H
#define ANDORCAMERA_ANDORDIRECT_DECL_H

#include <dStorm/input/fwd.h>
#include <dStorm/ImageTraits_decl.h>
#include <dStorm/engine/Image_decl.h>
#include <stdint.h>

namespace dStorm {
namespace AndorCamera {

typedef uint16_t CameraPixel;
typedef dStorm::Image<CameraPixel,2> CamImage;
typedef dStorm::input::Source< dStorm::engine::Image > CamSource;
class CamConfig;
typedef dStorm::input::Traits< dStorm::engine::Image > CamTraits;

class Method;
class Source;

}
}

#endif
