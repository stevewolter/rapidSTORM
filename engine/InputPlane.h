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
    bool has_background_estimate;
    const traits::Projection& projection() const { return *projection_; }
    void create_projection();

    InputPlane() : has_background_estimate(false) {}
    ~InputPlane();

private:
    boost::shared_ptr< traits::Projection > projection_;
};

}
}

#endif
