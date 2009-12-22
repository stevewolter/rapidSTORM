#include "BinnedLocalizations.h"
#include <dStorm/output/ResultRepeater.h>
#include <dStorm/input/ImageTraits.h>
#include <Eigen/Array>

#define LINEAR
// #define QUADRATIC

namespace dStorm {
namespace outputs {

template <typename KeepUpdated>
BinnedLocalizations<KeepUpdated>::BinnedLocalizations
    (double res_enh, int crop)
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

    for (int i = 0; i < er.number; i++) {
        const Localization& l = er.first[i];
        /* Find coordinates in resolution-enhanced space. */
        Eigen::Vector2d pos_in_im = 
            re * (l.position().start<2>().cwise() - crop);

        int lx = floor(pos_in_im.x()), ly = floor(pos_in_im.y());
        if (   lx < 0 || lx+1 >= int(base_image.width) ||
               ly < 0 || ly+1 >= int(base_image.height) )
            continue;
        float xf = pos_in_im.x() - lx, yf = pos_in_im.y() - ly;

        float strength = l.strength();

        this->binningListener().announce( l );

        /* This loops iterates over the four linear interpolation
            * terms (1-xf)*(1-yf), ... */
        for (int dx = 0; dx <= 1; dx++)
            for (int dy = 0; dy <= 1; dy++) 
            {
                int xp = lx+(1-dx), yp = ly+(1-dy);
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
    Eigen::Matrix<double,Localization::Dim,1> temp_size =
        (announcement->traits.size.cwise() - (1+2*crop))
            .cast<double>() * re;
    traits.size.fill( 1 );
    for (int i = 0; i < std::min(temp_size.rows(), traits.size.rows());i++)
        traits.size[i] = 
            ceil( std::max<double>(0, temp_size[i]) ) + 1;
    traits.resolution.start<Localization::Dim>()
        = announcement->traits.resolution / re;
    traits.resolution.end<3-Localization::Dim>().fill( 1 );

    base_image.resize(traits.dimx(),traits.dimy(),traits.dimz(),
                      1, /* No init */ -1);
    base_image.fill(0); 
    this->binningListener().setSize(traits);
}

}
}
