#ifndef DSTORM_DENSITY_MAP_LISTENER_H
#define DSTORM_DENSITY_MAP_LISTENER_H

#include <dStorm/image/fwd.h>

namespace dStorm {
namespace density_map {

/** Public interface necessary for a class listening
    *  to \c BinnedLocalizations. */
template <int Dimensions>
class Listener {
public:
    typedef dStorm::Image<float,Dimensions> BinnedImage;
    typedef dStorm::image::MetaInfo<Dimensions> MetaInfo;

    /** The setSize method is called before any announcements are made,
        *  and is called afterwards when the image size changes.
        *  @param traits     Traits of the binned image
        */
    inline void setSize(const MetaInfo&) ;
    /** This method is a forward for dStorm::Output method 
        *  announceStormSize. */
    inline void announce(const output::Output::Announcement&) ;
    /** This method is called when processing of an engine result 
        *  starts. */
    inline void announce(const output::Output::EngineResult&) ;
    /** This method is called once for each localization processed. */
    inline void announce(const Localization&) ;
    /** Called when a pixel changes in the binned image. The parameters
        *  give the x and y position of the changed pixel and its old and
        *  new value. */
    inline void updatePixel(const typename BinnedImage::Position&, float, float) ;
    /** Forwards the call to BinnedLocalizations::clean(), that is,
        *  the listener should clean its state.
        *  @param lastClean    This clean is the final clean after the
        *                      whole image was constructed */
    inline void clean( bool lastClean ) ;
    /** Forwards the call to BinnedLocalizations::clear(), that is,
        *  the state should be reset to an empty image. */
    inline void clear() ;
};

}
}

#endif
