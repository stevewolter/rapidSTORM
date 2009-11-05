#include "BinnedLocalizations.h"
#include <dStorm/output/ResultRepeater.h>
#include <dStorm/input/ImageTraits.h>
#include <Eigen/Array>

#define LINEAR
// #define QUADRATIC

namespace dStorm {

template <typename KeepUpdated>
BinnedLocalizations<KeepUpdated>::BinnedLocalizations
    (double res_enh, int crop)
    : Object("BinnedLocalizations", ""),
        crop(crop)
    { set_resolution_enhancement(res_enh); }

template <typename KeepUpdated>
BinnedLocalizations<KeepUpdated>::BinnedLocalizations
    (const BinnedLocalizations& o)
: Object(o), re(o.re), crop(o.crop), r(o.r), base_image(o.base_image),
  announcement( 
    (o.announcement.get() == NULL )
        ? NULL : new Announcement(*o.announcement) )
{}

template <typename KeepUpdated>
Output::AdditionalData
BinnedLocalizations<KeepUpdated>
::announceStormSize(const Announcement& a)
{
    ost::MutexLock lock(mutex);
    announcement.reset( new Announcement(a) );
    set_base_image_size();
    this->binningListener().announce( a );
    return NoData;
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
Output::Result 
BinnedLocalizations<KeepUpdated>
::receiveLocalizations(const EngineResult& er)
{
    ost::MutexLock lock(mutex);
    this->binningListener().announce(er);

    for (int i = 0; i < er.number; i++) {
        const Localization& l = er.first[i];
        /* Find coordinates in resolution-enhanced space. */
        int lx = l.getXLow(r)-crop*re, ly = l.getYLow(r)-crop*re;
        if (   lx < 0 || lx+1 >= int(base_image.width) ||
               ly < 0 || ly+1 >= int(base_image.height) )
            continue;
        float xf = l.getXR(r), yf = l.getYR(r);

        float strength = l.getStrength();

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
    r = dStorm::Localization::getRaster(re);

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
    CImgBuffer::Traits<BinnedImage> traits;
    Eigen::Vector3d temp_size =
        (announcement->traits.size.cwise() - (1+2*crop))
            .cast<double>() * re;
    for (int i = 0; i < traits.size.rows(); i++)
        traits.size[i] = 
            ceil( std::max<double>(0, temp_size[i]) ) + 1;
    traits.resolution = announcement->traits.resolution / re;
    traits.dim = announcement->traits.dim;

    base_image.resize(traits.dimx(),traits.dimy(),traits.dimz(),
                      traits.dim, /* No init */ -1);
    base_image.fill(0); 
    this->binningListener().setSize(traits);
}

};
