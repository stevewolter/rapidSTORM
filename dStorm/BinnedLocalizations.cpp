#include "BinnedLocalizations.h"
#include "HighDepthImage.h"

#define LINEAR
// #define QUADRATIC

namespace dStorm {

Transmission::AdditionalData
BinnedLocalizations::announceStormSize(int w, int h, int ) throw() {
    ost::MutexLock lock(mutex);
    int hrw = int(ceil(re*(w-1-2*crop)))+1, 
        hrh = int(ceil(re*(h-1-2*crop)))+1;
    base_image.resize(hrw,hrh,1,1);
    base_image.fill(0); 
    if (keepUpdated) keepUpdated->setSize(hrw, hrh);
    return NoData;
}

Transmission::Result 
BinnedLocalizations::receiveLocalizations(const EngineResult& er) throw()
{
    ost::MutexLock lock(mutex);
    for (int i = 0; i < er.number; i++) {
        if (er.first[i].getStrength() > ampThres) {
            int lx = er.first[i].getXLow(r)-crop*re, 
                ly = er.first[i].getYLow(r)-crop*re;
            float xf = er.first[i].getXR(r), yf = er.first[i].getYR(r);

#ifdef QUADRATIC
            float valArray[2][2];
            float valSum =0;
#endif

            /* This loops iterates over the four linear interpolation
                * terms (1-xf)*(1-yf), ... */
            for (int dx = 0; dx <= 1; dx++)
                for (int dy = 0; dy <= 1; dy++) 
                {
                    int xp = lx+(1-dx), yp = ly+(1-dy);
                    if (xp >= int(base_image.width) || 
                        yp >= int(base_image.height) ) continue;
#ifdef LINEAR   /* Linearly interpolating */
                    float val = (dx+(1-2*dx)*xf)*(dy+(1-2*dy)*yf);
                    base_image(xp, yp) += val;
#else
#ifdef QUADRATIC
                    float val = (dx+(1-2*dx)*xf*xf)*(dy+(1-2*dy)*yf*yf);
                    valArray[dx][dy] = val;
                    valSum += val;
                }
            for (int dx = 0; dx <= 1; dx++)
                for (int dy = 0; dy <= 1; dy++) 
                {
                    int xp = lx+(1-dx), yp = ly+(1-dy);
                    base_image(xp,yp) += valArray[dx][dy] / valSum;
#endif
#endif
                    if (keepUpdated)
                      keepUpdated->updatePixel(xp, yp, base_image(xp,yp));
                }
        }
    }
    return KeepRunning;
}

void BinnedLocalizations::clean() throw() {
    if (keepUpdated)
        keepUpdated->clean(base_image);
}

void BinnedLocalizations::clear() throw() {
    base_image.fill(0);
    if (keepUpdated)
        keepUpdated->clear();
}

};
