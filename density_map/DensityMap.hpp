#include <boost/units/systems/si/dimensionless.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/Eigen/Array>
#include <boost/units/io.hpp>

#include "base/Engine.h"
#include "image/iterator.h"
#include "image/contains.h"
#include "image/constructors.h"
#include "units/amplitude.h"

#include "density_map/DensityMap.h"
#include "density_map/Coordinates.h"

namespace dStorm {
namespace density_map {

template <typename Listener_, int Dim>
DensityMap<Listener_,Dim>::DensityMap(
    Listener_* listener,
    std::auto_ptr< Coordinates<Dim> > strategy, InterpolatorPtr interpolator, Crop crop
) : crop(crop), listener(listener), strategy(strategy), binningInterpolator(interpolator)
{
    assert( this->strategy.get() );
    assert( this->binningInterpolator.get() );
}

template <typename Listener, int Dim>
DensityMap<Listener,Dim>::DensityMap
    (const DensityMap& o)
: crop(o.crop), base_image(o.base_image),
  announcement( 
    (o.announcement.get() == NULL )
        ? NULL : new Announcement(*o.announcement) ),
  listener( o.listener ),
  strategy( o.strategy->clone() ),
  binningInterpolator(o.binningInterpolator->clone())
{}

template <typename Listener, int Dim>
void DensityMap<Listener,Dim>::set_base_image_size() 
{
    image::MetaInfo<Dim> traits;

    typedef quantity<camera::length>
        PreciseSize;
        
    const PreciseSize one_pixel( 1 * camera::pixel );

    traits.size.fill( 1 * camera::pixel );
    for (int i = 0; i < Dim; i++) {
        PreciseSize dp_size = strategy->get_size()[i] - 2*crop[i];
        traits.size[i] = std::max( ceil( dp_size), one_pixel );
    }

    traits.set_resolution( strategy->get_resolution() );

    base_image = BinnedImage(traits.size, base_image.frame_number());
    base_image.fill(0); 
    this->listener->setSize(traits);
}

template <typename Listener, int Dim>
output::Output::AdditionalData
DensityMap<Listener,Dim>
::announceStormSize(const Announcement& a)
{
    announcement.reset( new Announcement(a) );
    this->strategy->announce( a );
    this->listener->announce( a );
    set_base_image_size();
    return AdditionalData();
}

template <typename Listener, int Dim>
void DensityMap<Listener,Dim>
::store_results_( bool )
{
    this->listener->clean( true );
}

template <typename Listener, int Dim>
void
DensityMap<Listener,Dim>
::receiveLocalizations(const EngineResult& er)
{
    if ( er.size() == 0 ) return;
    this->listener->announce(er);

    typedef boost::units::quantity<camera::length,int> 
        pixel_count;

    typedef Eigen::Matrix< pixel_count, Dim, 1> Position;

    typename Coordinates<Dim>::Result r( er.size() );
    int point_count = strategy->bin_points(er, r);

    typedef std::vector< typename Interpolator<Dim>::ResultPoint > Points;
    Points points;
    for (int i = 0; i < point_count; i++) {
        const Localization& l = er[i];

        float strength = r[i].intensity;

        this->listener->announce( l );
        this->binningInterpolator->interpolate( r[i].position, r[i].position_uncertainty, points );

        for ( typename Points::const_iterator point = points.begin(); point != points.end(); ++point ) {
            const typename BinnedImage::Position p = point->position - crop;
            if ( ! base_image.contains( p ) ) continue;
            float val = strength * point->relative_value;
            float old_val = base_image(p);
            float new_val = (base_image(p) += val);
            this->listener->updatePixel( p, old_val, new_val );
        }
    }
}

template <typename Listener, int Dim>
void DensityMap<Listener,Dim>::clean() {
    this->listener->clean(false);
}

template <typename Listener, int Dim>
void DensityMap<Listener,Dim>::clear() {
    base_image.fill(0);
    this->listener->clear();
}

template <typename Listener, int Dim>
void DensityMap<Listener,Dim>::write_density_matrix( std::ostream& o ) 
{
    for ( typename BinnedImage::iterator i = base_image.begin(); i != base_image.end(); ++i )
        o << value( i.position() ).transpose() << " " << *i << "\n";
}

template <typename Listener, int Dim>
const typename DensityMap<Listener,Dim>::Crop
DensityMap<Listener,Dim>::no_crop
    = DensityMap<Listener,Dim>::Crop::Constant( 0 * boost::units::camera::pixel );

template <typename Listener, int Dim>
DensityMap<Listener,Dim>::~DensityMap() {}


}
}
