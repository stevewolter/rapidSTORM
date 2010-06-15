#include "BinnedLocalizations.h"
#include <dStorm/output/ResultRepeater.h>
#include <dStorm/ImageTraits.h>
#include <dStorm/image/iterator.h>
#include <Eigen/Array>
#include <boost/units/systems/si/dimensionless.hpp>
#include <boost/units/cmath.hpp>
#include <dStorm/unit_matrix_operators.h>
#include <dStorm/matrix_operators.h>
#include <dStorm/units/amplitude.h>
#include <boost/units/io.hpp>

#define LINEAR
// #define QUADRATIC

namespace dStorm {
namespace outputs {

template <typename KeepUpdated>
BinnedLocalizations<KeepUpdated>::BinnedLocalizations
    (double res_enh, Crop crop)
    : OutputObject("BinnedLocalizations", ""),
        crop(crop)
    { set_resolution_enhancement(res_enh); }

template <typename KeepUpdated>
BinnedLocalizations<KeepUpdated>::BinnedLocalizations
    (const BinnedLocalizations& o)
: OutputObject(o), re(o.re), crop(o.crop), base_image(o.base_image),
  announcement( 
    (o.announcement.get() == NULL )
        ? NULL : new Announcement(*o.announcement) )
{}

template <typename KeepUpdated>
output::Output::AdditionalData
BinnedLocalizations<KeepUpdated>
::announceStormSize(const Announcement& a)
{
    ost::MutexLock lock(mutex);
    announcement.reset( new Announcement(a) );
    set_base_image_size();
    this->binningListener().announce( a );
    return AdditionalData();
}

template <typename KeepUpdated>
void BinnedLocalizations<KeepUpdated>
::propagate_signal(ProgressSignal s)
{
    if ( s == Engine_is_restarted ) {
        ost::MutexLock lock(mutex); 
        clear();
    } else if ( s == Engine_run_succeeded ) {
        ost::MutexLock lock(mutex); 
        this->binningListener().clean( true );
    }
}

template <typename KeepUpdated>
output::Output::Result 
BinnedLocalizations<KeepUpdated>
::receiveLocalizations(const EngineResult& er)
{
    ost::MutexLock lock(mutex);
    this->binningListener().announce(er);

    typedef boost::units::quantity<cs_units::camera::length,int> 
        pixel_count;

    typedef Localization::Position::Scalar Offset;
    typedef Eigen::Matrix<Offset,2,1> Position;

    for (int i = 0; i < er.number; i++) {
        const Localization& l = er.first[i];
        /* Find coordinates in resolution-enhanced space. */
        Position orig = l.position().start<2>(),
                 cropped = orig.cwise() - crop,
                 enlarged = re * cropped;

        pixel_count lx = pixel_count(floor(enlarged.x())),
                    ly = pixel_count(floor(enlarged.y())),
                    onepix = 1 * cs_units::camera::pixel;
        float xf = (enlarged.x() - lx) / cs_units::camera::pixel,
              yf = (enlarged.y() - ly) / cs_units::camera::pixel;

        amplitude strength = l.strength();

        this->binningListener().announce( l );

        /* This loops iterates over the four linear interpolation
            * terms (1-xf)*(1-yf), ... */
        for (int dx = 0; dx <= 1; dx++)
            for (int dy = 0; dy <= 1; dy++) 
            {
                int xp = lx.value()+(1-dx), yp = ly.value()+(1-dy);
                if ( xp < 0 || yp < 0
                        || xp >= base_image.width_in_pixels()
                        || yp >= base_image.height_in_pixels() )
                  continue;
                float val = strength / cs_units::camera::ad_count
                                     * (dx+(1-2*dx)*xf)*(dy+(1-2*dy)*yf);
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
    ost::MutexLock lock(mutex);
    this->binningListener().clean(false);
}

template <typename KeepUpdated>
void BinnedLocalizations<KeepUpdated>::clear() {
    ost::MutexLock lock(mutex);
    base_image.fill(0);
    this->binningListener().clear();
}

template <typename KeepUpdated>
void BinnedLocalizations<KeepUpdated>::
    set_resolution_enhancement(double re) 
 
{
    ost::MutexLock lock(mutex);
    this->re = re;

    if ( announcement.get() != NULL && 
         announcement->result_repeater != NULL )
    {
        set_base_image_size();
        announcement->result_repeater->repeat_results();
    }
}

template <typename KeepUpdated>
void BinnedLocalizations<KeepUpdated>::set_base_image_size() 
 
{
    input::Traits<BinnedImage> traits;

    typedef boost::units::quantity<cs_units::camera::length,float>
        PreciseSize;
        
    const PreciseSize one_pixel( 1 * cs_units::camera::pixel );

    traits.size.fill( 1 * cs_units::camera::pixel );
    for (int i = 0; i < announcement->traits.size.rows(); i++) {
        PreciseSize dp_size = announcement->traits.size[i] - 2*crop;
        traits.size[i] = std::max( ceil( re * (dp_size - one_pixel) + one_pixel ), one_pixel );
    }

    if ( announcement->traits.resolution.is_set() )
        traits.resolution = *announcement->traits.resolution * float(re);

    base_image = BinnedImage(traits.size, base_image.frame_number());
    base_image.fill(0); 
    this->binningListener().setSize(traits);
}

}
}
