#include "InputTraits.h"
#include <dStorm/traits/Projection.h>

namespace dStorm {
namespace input {

void Traits< engine::ImageStack >::push_back( const image::MetaInfo<2>& mi, const traits::Optics& o )
{
    planes_.push_back( engine::InputPlane() );
    planes_.back().image = mi;
    planes_.back().optics = o;
}

void Traits< engine::ImageStack >::push_back( const engine::InputPlane& p )
{
    planes_.push_back(p);
}

void Traits< engine::ImageStack >::clear()
{
    planes_.clear();
}

Traits< engine::ImageStack >::Traits() {}

Traits< engine::ImageStack >::Traits( const image::MetaInfo<2>& i ) 
{
    push_back( i, traits::Optics() );
}

samplepos Traits< engine::ImageStack >
::size_in_sample_space() const
{
    samplepos p = samplepos::Constant(0.0f * si::meter);
    for (int pl = 0; pl < this->plane_count(); ++pl) {
        image::MetaInfo<2>::Size size = plane(pl).image.size;
        size.array() -= 1 * camera::pixel;
        traits::Projection::SamplePosition xy
            = this->plane(pl).projection().pixel_in_sample_space( size );
        p.x() = std::max( p.x(), xy.x() );
        p.y() = std::max( p.y(), xy.y() );
        p.z() = std::max( p.z(), 
                          samplepos::Scalar(*plane(pl).optics.z_position) );
    }
    return p;
}

}
}
