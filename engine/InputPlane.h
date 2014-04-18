#ifndef DSTORM_ENIGNE_INPUTPLANE_H
#define DSTORM_ENIGNE_INPUTPLANE_H

#include "traits/optics.h"
#include "image/MetaInfo.h"

namespace dStorm {
namespace traits { class Projection; }
namespace engine {

struct InputPlane {
    image::MetaInfo<2> image;
    traits::Optics optics;
    const traits::Projection& projection() const { return *projection_; }
    void create_projection();

    ~InputPlane();

private:
    boost::shared_ptr< traits::Projection > projection_;
};

}
}

#endif
