#include "BinnedLocalizations.h"
#include "../Engine.h"
#include "../ImageTraits.h"
#include "../image/iterator.h"
#include <Eigen/Core>
#include <boost/units/systems/si/dimensionless.hpp>
#include <boost/units/cmath.hpp>
#include <boost/units/Eigen/Array>
#include "../units/amplitude.h"
#include <boost/units/io.hpp>

#define LINEAR
// #define QUADRATIC

namespace dStorm {
namespace outputs {

template <typename KeepUpdated>
BinnedLocalizations<KeepUpdated>::BinnedLocalizations
    (std::auto_ptr<BinningStrategy> strategy, Crop crop)
    : OutputObject("BinnedLocalizations", ""),
        crop(crop), strategy(strategy)
    {}

template <typename KeepUpdated>
BinnedLocalizations<KeepUpdated>::BinnedLocalizations
    (const BinnedLocalizations& o)
: OutputObject(o), crop(o.crop), base_image(o.base_image),
  announcement( 
    (o.announcement.get() == NULL )
        ? NULL : new Announcement(*o.announcement) ),
  strategy( o.strategy->clone() )
{}

template <typename KeepUpdated>
output::Output::AdditionalData
BinnedLocalizations<KeepUpdated>
::announceStormSize(const Announcement& a)
{
    announcement.reset( new Announcement(a) );
    this->strategy->announce( a );
    this->binningListener().announce( a );
    set_base_image_size();
    return AdditionalData();
}

template <typename KeepUpdated>
void BinnedLocalizations<KeepUpdated>
::propagate_signal(ProgressSignal s)
{
    if ( s == Engine_is_restarted ) {
        clear();
    } else if ( s == Engine_run_succeeded ) {
        this->binningListener().clean( true );
    }
}

template <typename KeepUpdated>
output::Output::Result 
BinnedLocalizations<KeepUpdated>
::receiveLocalizations(const EngineResult& er)
{
    if ( er.size() == 0 ) return KeepRunning;
    this->binningListener().announce(er);

    typedef boost::units::quantity<camera::length,int> 
        pixel_count;

    typedef Eigen::Matrix< pixel_count,2,1> Position;

    BinningStrategy::Result r( er.size(), 3 );
    strategy->bin_points(er, r);

    for (size_t i = 0; i < er.size(); i++) {
        const Localization& l = er[i];

        float lx = (floor(r(i,0))), ly = (floor(r(i,1)));
        float xf = (r(i,0) - lx),
              yf = (r(i,1) - ly);
        float strength = r(i,2);

        this->binningListener().announce( l );

        /* This loops iterates over the four linear interpolation
            * terms (1-xf)*(1-yf), ... */
        for (int dx = 0; dx <= 1; dx++)
            for (int dy = 0; dy <= 1; dy++) 
            {
                int xp = lx+(1-dx)-crop.value(), yp = ly+(1-dy)-crop.value();
                if ( xp < 0 || yp < 0
                        || xp >= base_image.width_in_pixels()
                        || yp >= base_image.height_in_pixels() )
                  continue;
                float val = strength * (dx+(1-2*dx)*xf)*(dy+(1-2*dy)*yf);
                float old_val = base_image(xp, yp);

                base_image(xp, yp) += val;
                this->binningListener().updatePixel
                    ( xp, yp, old_val, base_image(xp,yp) );
            }
    }
    return KeepRunning;
}

template <typename KeepUpdated>
void BinnedLocalizations<KeepUpdated>::clean() {
    this->binningListener().clean(false);
}

template <typename KeepUpdated>
void BinnedLocalizations<KeepUpdated>::clear() {
    base_image.fill(0);
    this->binningListener().clear();
}

template <typename KeepUpdated>
void BinnedLocalizations<KeepUpdated>::set_base_image_size() 
 
{
    input::Traits<BinnedImage> traits;

    typedef quantity<camera::length>
        PreciseSize;
        
    const PreciseSize one_pixel( 1 * camera::pixel );

    traits.size.fill( 1 * camera::pixel );
    for (int i = 0; i < 2; i++) {
        PreciseSize dp_size = strategy->get_size()[i] - 2*crop;
        traits.size[i] = std::max( ceil( dp_size), one_pixel );
    }

    traits.set_resolution( strategy->get_resolution() );

    base_image = BinnedImage(traits.size, base_image.frame_number());
    base_image.fill(0); 
    this->binningListener().setSize(traits);
}

}
}
