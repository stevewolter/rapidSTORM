#include "InputTraits.h"
#include <dStorm/traits/Projection.h>
#include <boost/variant/get.hpp>
#include <boost/units/Eigen/Core>
#include <boost/units/Eigen/Array>
#include <dStorm/units/microlength.h>

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
        z_range += optics(pl).depth_info()->z_range();
    }
    if ( ! is_empty( z_range ) ) {
        min.z() = lower( z_range );
        max.z() = upper( z_range );
    }
    return std::make_pair( min, max );
}

#if 0
class print_threed_info 
: public boost::static_visitor<void>
{
    std::ostream& o;
public:
    print_threed_info( std::ostream& target ) : o(target) {}
    void operator()( const threed_info::Polynomial3D& p ) const {
        o << "polynomial 3D with X focus depths " ;
        for (int j = threed_info::Polynomial3D::MinTerm; j <= threed_info::Polynomial3D::Order; ++j)
            o << 1.0 / p.get_slope(Direction_X, j) << " ";
        o << " and Y focus depth " ;
        for (int j = threed_info::Polynomial3D::MinTerm; j <= threed_info::Polynomial3D::Order; ++j)
            o << 1.0 / p.get_slope(Direction_Y, j) << " ";
        o << " and focal planes " << p.focal_planes()->transpose();
    }
};
#endif

std::ostream& Traits< engine::ImageStack >::print_psf_info( std::ostream& o ) const {
    for ( int j = 0; j < plane_count(); ++j) {
        const traits::Optics& optics = this->optics(j);
        if ( j != 0 ) o << ", ";
        o << "plane " << j << " has " << *optics.depth_info();
        for ( size_t i = 0; i < fluorophores.size(); ++i )
        {
            traits::Optics::PSF psf = *optics.psf_size(i);
            for (Direction dir = Direction_X; dir != Direction_2D; ++dir)
                psf[dir] *= 2.35;
            o << ", fluorophore " << i << " has PSF FWHM " 
                    << psf.cast< quantity<si::microlength> >().transpose()
                    << " and transmission " << optics.transmission_coefficient(i);
        }
    }
    return o;
}


}
}
