#ifndef DSTORM_ENIGNE_INPUTPLANE_H
#define DSTORM_ENIGNE_INPUTPLANE_H

#include <dStorm/traits/optics.h>
#include <dStorm/traits/Projection.h>
#include <dStorm/image/MetaInfo.h>

namespace dStorm {
namespace engine {

struct InputPlane {
    image::MetaInfo<2> image;
    traits::Optics optics;
    const traits::Projection& projection() const { return *projection_; }
    void start_job();
private:
    boost::shared_ptr< traits::Projection > projection_;
};

}
}

#endif
