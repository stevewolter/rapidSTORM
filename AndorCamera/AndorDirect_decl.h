#ifndef ANDORCAMERA_ANDORDIRECT_DECL_H
#define ANDORCAMERA_ANDORDIRECT_DECL_H

#include <dStorm/input/Source_decl.h>
#include <dStorm/input/ImageTraits_decl.h>
#include <dStorm/input/Method_decl.h>
#include <stdint.h>

namespace cimg_library {
    template <typename PixelType> class CImg;
};

namespace dStorm {
namespace AndorDirect {

typedef uint16_t CameraPixel;
typedef cimg_library::CImg<CameraPixel> CamImage;
typedef input::Source< CamImage > CamSource;
typedef input::Method<CamImage> CamConfig;
typedef input::Traits<CamImage> CamTraits;

class Config;
class Source;

}
}

#endif
