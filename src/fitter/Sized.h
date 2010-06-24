#ifndef DSTORM_FITTER_SIZED_H
#define DSTORM_FITTER_SIZED_H

#include <dStorm/Localization_decl.h>
#include <dStorm/engine/Spot_decl.h>
#include <dStorm/engine/Image_decl.h>

namespace dStorm {
namespace fitter {

typename <class BaseFitter>
struct Sized {
    virtual ~Sized() {}
    virtual int fit(const Spot& spot, Localization* target,
        const BaseImage &image, int xl, int yl ) = 0;
    virtual void setSize( int width, int height ) = 0;
};

}
}
#endif
