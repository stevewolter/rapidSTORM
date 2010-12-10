#ifndef DSTORM_SPOTFITTER_H
#define DSTORM_SPOTFITTER_H

#include "Image.h"
#include "Spot.h"
#include "Localization.h"
#include "Config.h"
#include <memory>

namespace dStorm {
   class SpotFitter {
      public:
         static std::auto_ptr<SpotFitter> factory(const Config &);
         virtual ~SpotFitter() throw() {}

         virtual void prepareToFitImage(const Image &im) = 0;
         virtual bool fitSpot( const Spot& spot, Localization *target ) throw() = 0;
   };
}

#endif
