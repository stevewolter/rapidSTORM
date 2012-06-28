#ifndef DSTORM_DENSITY_MAP_DUMMYLISTENER_H
#define DSTORM_DENSITY_MAP_DUMMYLISTENER_H

#include "Listener.h"

namespace dStorm {
namespace density_map {

/** A dummy class implementing all methods necessary for a Listener.
    *  All methods are empty so that, after inlining, no superfluous
    *  code is left in BinnedLocalizations for an empty listener
    *  slot. */
template <int Dimensions>
struct DummyListener : public Listener<Dimensions> {
    typedef dStorm::Image<float,Dimensions> BinnedImage;
    typedef typename Listener<Dimensions>::MetaInfo MetaInfo;
    void setSize(const MetaInfo&) {}
    void announce(const output::Output::Announcement&) {}
    void announce(const output::Output::EngineResult&) {}
    void announce(const Localization&) {}
    void updatePixel(const typename BinnedImage::Position&, float, float) {}
    void clean(bool) {}
    void clear() {}
};

}
}

#endif
