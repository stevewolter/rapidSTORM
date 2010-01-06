#include "BinnedLocalizations.h"
#include <dStorm/output/ResultRepeater.h>
#include <dStorm/input/ImageTraits.h>
#include <Eigen/Array>
#include <boost/units/systems/si/dimensionless.hpp>
#include <boost/units/cmath.hpp>
#include <dStorm/unit_matrix_operators.h>
#include <dStorm/matrix_operators.h>

#define LINEAR
// #define QUADRATIC

namespace dStorm {
namespace outputs {

template <typename KeepUpdated>
BinnedLocalizations<KeepUpdated>::BinnedLocalizations
    (double res_enh, pixel_count crop)
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

    typedef boost::units::quantity<camera::length,float>
        Offset;
    typedef Eigen::Matrix<Offset,2,1>          
        Position;

    for (int i = 0; i < er.number; i++) {
        const Localization& l = er.first[i];
        /* Find coordinates in resolution-enhanced space. */
        Position cropped 
            = (l.position().start<2>().cwise() - crop);
        Position enlarged = re * cropped;

        pixel_count lx = static_cast<pixel_count>
                            (floor(enlarged.x())),
                    ly = static_cast<pixel_count>
                            (floor(enlarged.y()));
        if (   lx < 0 * camera::pixel || 
               lx >= int(base_image.width-1) * camera::pixel ||
               ly < 0 * camera::pixel || 
               ly >= int(base_image.height-1) * camera::pixel )
            continue;
        float xf = (enlarged.x() - lx).value(), 
              yf = (enlarged.y() - ly).value();

        float strength = l.strength();

        this->binningListener().announce( l );

        /* This loops iterates over the four linear interpolation
            * terms (1-xf)*(1-yf), ... */
        for (int dx = 0; dx <= 1; dx++)
            for (int dy = 0; dy <= 1; dy++) 
            {
                int xp = lx.value()+(1-dx), 
                    yp = ly.value()+(1-dy);
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
    const pixel_count one_pixel( 1 * camera::pixel );

    typedef quantity<camera::length> Size;
    typedef Eigen::Matrix<Size,Localization::Dim,1>
        ImageSize;

    ImageSize size =
            (announcement->traits.size.cwise()
                - (one_pixel+crop+crop))
                .cast<Size>();
    size = (std::ceil(size)
        .cwise() + one_pixel)
        .cwise().max( ImageSize::Constant(one_pixel) );
                                 
    traits.size.fill( 1 * camera::pixel );
    traits.size.start<Localization::Dim>() 
        = size.cast<pixel_count>();
    traits.resolution.start<Localization::Dim>()
        = announcement->traits.resolution / float(re);
    traits.resolution.end<3-Localization::Dim>().fill
        ( 0*si::meter/camera::pixel );

    base_image.resize(
        traits.dimx().value(),
        traits.dimy().value(),
        traits.dimz().value(),
                      1, /* No init */ -1);
    base_image.fill(0); 
    this->binningListener().setSize(traits);
}

}
}
