#include "engine/InputTraits.h"
#include "traits/Projection.h"
#include <boost/variant/get.hpp>
#include "units/microlength.h"
#include "threed_info/DepthInfo.h"

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

std::pair<samplepos,samplepos> Traits< engine::ImageStack >
::size_in_sample_space() const
{
    samplepos min = samplepos::Constant(std::numeric_limits<float>::max() * si::meter);
    samplepos max = samplepos::Constant(-std::numeric_limits<float>::max() * si::meter);
    threed_info::ZRange z_range;
    for (int pl = 0; pl < this->plane_count(); ++pl) {
        image::MetaInfo<2>::Size size = plane(pl).image.size;
        size.array() -= 1 * camera::pixel;
        traits::Projection::SamplePosition xy
            = this->plane(pl).projection().pixel_in_sample_space( size );
        min[0] = std::min( min[0], 0.0f * si::meter );
        min[1] = std::min( min[1], 0.0f * si::meter );
        max[0] = std::max( max[0], xy[0] );
        max[1] = std::max( max[1], xy[1] );
        for (Direction d = Direction_First; d != Direction_2D; ++d)
            z_range += optics(pl).depth_info(d)->z_range();
    }
    if ( ! is_empty( z_range ) ) {
        min.z() = lower( z_range ) * 1E-6 * si::meter;
        max.z() = upper( z_range ) * 1E-6 * si::meter;
    }
    return std::make_pair( min, max );
}

std::ostream& Traits< engine::ImageStack >::print_psf_info( std::ostream& o ) const {
    for ( int j = 0; j < plane_count(); ++j) {
        const traits::Optics& optics = this->optics(j);
        if ( j != 0 ) o << ", ";
        o << "plane " << j << " has " << *optics.depth_info(Direction_X) << " in X and "
          << *optics.depth_info(Direction_Y) << " in Y";
        for ( size_t i = 0; i < fluorophores.size(); ++i )
        {
            o << ", fluorophore " << i << " has transmission "
              << optics.transmission_coefficient(i);
        }
    }
    return o;
}


}
}
