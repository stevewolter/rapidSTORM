#ifndef DSTORM_SPOTFITTER_H
#define DSTORM_SPOTFITTER_H

#include <dStorm/engine/Image_decl.h>
#include <dStorm/engine/Spot.h>
#include <dStorm/Localization.h>
#include <memory>

namespace dStorm {
namespace engine {
   /** Base class for spot fitting classes. Users should call
    *  prepareToFitImage() for each new image that should be fitted,
    *  and then fitSpot() for each position in that image that should
    *  be fitted. */
   class SpotFitter {
      public:
         virtual ~SpotFitter() {}

         /** \param spot        Position at which to fit
          *  \param im          Image to be fitted.
          *  \param number      Number of the image to be fitted.
          *  \param target      Write results to this address
          *  \return            Number of found spots */
         virtual int fitSpot( const Spot& spot, const Image &im,
                              Localization *target ) = 0;
   };
}
}

#endif
