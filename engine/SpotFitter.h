#ifndef DSTORM_SPOTFITTER_H
#define DSTORM_SPOTFITTER_H

#include <dStorm/engine/Image.h>
#include <dStorm/engine/Spot.h>
#include <dStorm/engine/Localization.h>
#include <memory>

namespace dStorm {
   class Config;

   /** Base class for spot fitting classes. Users should call
    *  prepareToFitImage() for each new image that should be fitted,
    *  and then fitSpot() for each position in that image that should
    *  be fitted. */
   class SpotFitter {
      public:
         static std::auto_ptr<SpotFitter> factory(const Config &);
         virtual ~SpotFitter() {}

         /** \param spot        Position at which to fit
          *  \param im          Image to be fitted.
          *  \param number      Number of the image to be fitted.
          *  \param target      Write results to this address
          *  \return            Number of found spots */
         virtual int fitSpot( const Spot& spot, const Image &im,
                              int number, Localization *target )
                            = 0;
   };
}

#endif
