#ifndef DSTORM_GUF_DATAPLANE_IMPL_H
#define DSTORM_GUF_DATAPLANE_IMPL_H

#include "FittingRegion.h"
#include <memory>
#include <dStorm/engine/Image_decl.h>

namespace dStorm {
namespace fit_window {

template <typename Tag>
struct FittingRegionImpl
: public FittingRegion {
    std::auto_ptr<Centroid> _residue_centroid() const;

  public:
    FittingRegionImpl( 
        const Optics& input,
        const engine::Image2D& image,
        const Spot& position );

    typename Tag::Data data;

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif