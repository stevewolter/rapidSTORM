#ifndef DSTORM_GUF_DATAPLANE_IMPL_H
#define DSTORM_GUF_DATAPLANE_IMPL_H

#include "DataPlane.h"
#include "Statistics.h"
#include <memory>

namespace dStorm {
namespace guf {

template <typename Tag>
struct DataPlaneImpl
: public DataPlane {
    typename Tag::Data data;
    Statistics<2> image_stats;
    const void* get_data() const { return &data; }
    std::auto_ptr<Centroid> _residue_centroid() const;
    quantity< si::area > pixel_size() const 
        { return quantity< si::area >(data.pixel_size); }
    const Statistics<2>& get_statistics() const { return image_stats; }

  public:
    DataPlaneImpl( 
        const Optics& input,
        const dStorm::engine::Image2D& image,
        const guf::Spot& position );
    const typename Tag::Data& get_the_data() const { return data; }

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

}
}

#endif
