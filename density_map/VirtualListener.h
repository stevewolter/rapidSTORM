#ifndef DSTORM_DENSITY_MAP_VIRTUALLISTENER_H
#define DSTORM_DENSITY_MAP_VIRTUALLISTENER_H

#include <dStorm/image/fwd.h>
#include "density_map/Listener.h"

namespace dStorm {
namespace density_map {

/** Public interface necessary for a class listening
    *  to \c BinnedLocalizations. */
template <int Dimensions>
class VirtualListener : public Listener<Dimensions> {
public:
    typedef typename Listener<Dimensions>::MetaInfo MetaInfo;
    typedef typename Listener<Dimensions>::BinnedImage BinnedImage;

    virtual ~VirtualListener() {}
    virtual void setSize(const MetaInfo&) = 0;
    virtual void announce(const output::Output::Announcement&) = 0;
    virtual void announce(const output::Output::EngineResult&) = 0;
    virtual void announce(const Localization&) = 0;
    virtual void updatePixel(const typename BinnedImage::Position&, float, float) = 0;
    virtual void clean( bool lastClean ) = 0;
    virtual void clear() = 0;
};

}
}

#endif
