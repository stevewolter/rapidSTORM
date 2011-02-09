#ifndef DSTORM_FITTER_SIZED_H
#define DSTORM_FITTER_SIZED_H

#include <dStorm/Localization_decl.h>
#include <dStorm/engine/Spot_decl.h>
#include <dStorm/engine/Image_decl.h>

namespace dStorm {
namespace fitter {

struct Sized {
    virtual ~Sized() {}
    virtual int fit(
        const engine::Spot& spot, Localization* target,
        const engine::Image &image, int xl, int yl ) = 0;
    virtual void setSize( int width, int height ) = 0;
};

}
}
#endif
