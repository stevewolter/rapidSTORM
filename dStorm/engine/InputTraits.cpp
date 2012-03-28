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
                          std::max( 
                            samplepos::Scalar( (*plane(pl).optics.z_position)[0] ),
                            samplepos::Scalar( (*plane(pl).optics.z_position)[1] ) ) );
    }
    return p;
}

std::ostream& Traits< engine::ImageStack >::print_psf_info( std::ostream& o ) const {
    for ( int j = 0; j < plane_count(); ++j) {
        const traits::Optics& optics = this->optics(j);
        if ( j != 0 ) o << ", ";
        o << "plane " << j << " has";
        if ( const traits::Polynomial3D* p = boost::get< traits::Polynomial3D >(optics.depth_info().get_ptr()) ) {
            o << " X focus depths " ;
            for (int j = traits::Polynomial3D::MinTerm; j <= traits::Polynomial3D::Order; ++j)
                o << 1.0 / p->get_slope(Direction_X, j) << " ";
            o << " and Y focus depth " ;
            for (int j = traits::Polynomial3D::MinTerm; j <= traits::Polynomial3D::Order; ++j)
                o << 1.0 / p->get_slope(Direction_Y, j) << " ";
            o << " and focal planes " << optics.z_position->transpose();
        } else
            o << " no 3D information";
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
