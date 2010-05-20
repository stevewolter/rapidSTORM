#ifndef ANDORCAMERA_ANDORDIRECT_DECL_H
#define ANDORCAMERA_ANDORDIRECT_DECL_H

#include <dStorm/input/Source_decl.h>
#include <dStorm/ImageTraits_decl.h>
#include <dStorm/input/Method_decl.h>
#include <stdint.h>

namespace AndorCamera {

typedef uint16_t CameraPixel;
typedef dStorm::Image<CameraPixel,2> CamImage;
typedef dStorm::input::Source< CamImage > CamSource;
typedef dStorm::input::Method<CamImage> CamConfig;
typedef dStorm::input::Traits<CamImage> CamTraits;

class Method;
class Source;

}

#endif
