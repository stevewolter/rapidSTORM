#include "BinnedLocalizations.h"
#include <dStorm/ResultRepeater.h>

#define LINEAR
// #define QUADRATIC

namespace dStorm {

template <typename KeepUpdated>
BinnedLocalizations<KeepUpdated>::BinnedLocalizations
    (double res_enh, int crop)
    : Object("BinnedLocalizations", ""),
        crop(crop),
        announcement(0, 0, 0)
    { set_resolution_enhancement(res_enh); }

template <typename KeepUpdated>
Output::AdditionalData
BinnedLocalizations<KeepUpdated>
::announceStormSize(const Announcement& a)

{
    ost::MutexLock lock(mutex);
    announcement = a;
    set_base_image_size();
    this->binningListener().announce( a );
    return NoData;
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

        /* Reduce the strength to make better use of floating-point
         * exponent range. */
        float strength = l.getStrength() / 1000;

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
    this->binningListener().clean();
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

    if ( announcement.width != 0 && 
         announcement.result_repeater != NULL )
    {
        set_base_image_size();
        announcement.result_repeater->repeat_results();
    }
}

template <typename KeepUpdated>
void BinnedLocalizations<KeepUpdated>::set_base_image_size() 
 
{
    int w = announcement.width, h = announcement.height;
    int hrw = int(ceil(re*(w-1-2*crop)))+1, 
        hrh = int(ceil(re*(h-1-2*crop)))+1;
    base_image.resize(hrw,hrh,1,1, /* No init */ -1);
    base_image.fill(0); 
    this->binningListener().setSize(hrw, hrh);
}

};
