#ifndef DEBUG
#include "debug.h"
#endif
#include "BinnedLocalizations.h"
#include "../Engine.h"
#include "../ImageTraits.h"
#include "../image/iterator.h"
#include <dStorm/image/contains.h>
#include <Eigen/Core>
#include <boost/units/systems/si/dimensionless.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/Eigen/Array>
#include "../units/amplitude.h"
#include <boost/units/io.hpp>

namespace dStorm {
namespace outputs {

template <typename KeepUpdated, int Dim>
BinnedLocalizations<KeepUpdated,Dim>::BinnedLocalizations
    (std::auto_ptr<BinningStrategy<Dim> > strategy, Crop crop)
    : OutputObject("BinnedLocalizations", ""),
        crop(crop), strategy(strategy)
    {}

template <typename KeepUpdated, int Dim>
BinnedLocalizations<KeepUpdated,Dim>::BinnedLocalizations
    (const BinnedLocalizations& o)
: OutputObject(o), crop(o.crop), base_image(o.base_image),
  announcement( 
    (o.announcement.get() == NULL )
        ? NULL : new Announcement(*o.announcement) ),
  strategy( o.strategy->clone() )
{}

template <typename KeepUpdated, int Dim>
output::Output::AdditionalData
BinnedLocalizations<KeepUpdated,Dim>
::announceStormSize(const Announcement& a)
{
    announcement.reset( new Announcement(a) );
    this->strategy->announce( a );
    this->binningListener().announce( a );
    set_base_image_size();
    return AdditionalData();
}

template <typename KeepUpdated, int Dim>
void BinnedLocalizations<KeepUpdated,Dim>
::store_results()
{
    this->binningListener().clean( true );
}

template <typename KeepUpdated, int Dim>
void
BinnedLocalizations<KeepUpdated,Dim>
::receiveLocalizations(const EngineResult& er)
{
    if ( er.size() == 0 ) return;
    this->binningListener().announce(er);

    typedef boost::units::quantity<camera::length,int> 
        pixel_count;

    typedef Eigen::Matrix< pixel_count, Dim, 1> Position;

    typename BinningStrategy<Dim>::Result r( er.size(), Dim+1 );
    strategy->bin_points(er, r);

    for (size_t i = 0; i < er.size(); i++) {
        const Localization& l = er[i];

        Eigen::Array<float,Dim,1> values, lower, terms;
        values = r.row(i).template head<Dim>();
        lower = floor( values );
        float strength = r(i,Dim);

        this->binningListener().announce( l );

        const typename BinnedImage::Position base_pos 
            = lower.template cast<int>() - crop.value();

        /* This loops iterates over the linear interpolation terms,
         * i.e. the corners of the hypercube. */
        for (unsigned int corner_ = 0; corner_ < (1u << Dim); ++corner_) {
            typename BinnedImage::Position p = base_pos;
            std::bitset<Dim> high_corner( corner_ );
            for (int j = 0; j < Dim; ++j)
                if ( high_corner[j] ) p[j] += 1;
            if ( ! contains( base_image, p ) ) continue;
            for (int j = 0; j < Dim; ++j)
                    terms[j] = (high_corner[j]) ? (values[j] - lower[j]) : 1.0f - (values[j] - lower[j]);

            float val = strength * std::abs( terms.prod() );
            float old_val = base_image(p);
            float new_val = (base_image(p) += val);
            this->binningListener().updatePixel( p, old_val, new_val );
        }
    }
}

template <typename KeepUpdated, int Dim>
void BinnedLocalizations<KeepUpdated,Dim>::clean() {
    this->binningListener().clean(false);
}

template <typename KeepUpdated, int Dim>
void BinnedLocalizations<KeepUpdated,Dim>::clear() {
    base_image.fill(0);
    this->binningListener().clear();
}

template <typename KeepUpdated, int Dim>
void BinnedLocalizations<KeepUpdated,Dim>::set_base_image_size() 
 
{
    input::Traits<BinnedImage> traits;

    typedef quantity<camera::length>
        PreciseSize;
        
    const PreciseSize one_pixel( 1 * camera::pixel );

    traits.size.fill( 1 * camera::pixel );
    for (int i = 0; i < Dim; i++) {
        PreciseSize dp_size = strategy->get_size()[i] - 2*crop;
        traits.size[i] = std::max( ceil( dp_size), one_pixel );
    }
    DEBUG("Size of binned image is " << traits.size.transpose());

    traits.plane(0).set_resolution( strategy->get_resolution() );

    base_image = BinnedImage(traits.size, base_image.frame_number());
    base_image.fill(0); 
    this->binningListener().setSize(traits);
}

}
}
