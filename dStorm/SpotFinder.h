#ifndef DSTORM_SPOTFINDER_H
#define DSTORM_SPOTFINDER_H

#include "Image.h"
#include "Spot.h"
#include "CandidateTree.h"
#include <memory>
#include "Config.h"

namespace dStorm {
   class SpotFinder {
      protected:
        SpotFinder(const Config &conf, int imw, int imh) throw();
        const int msx, msy, bx, by, imw, imh;
        std::auto_ptr<SmoothedImage> smoothed;

      public:
        virtual ~SpotFinder() throw() {}

        static std::auto_ptr<SpotFinder> factory
            (const Config &conf, int imw, int imh) throw();

        virtual void prepare( const Image &image ) throw() = 0;

        inline void findSpots( const Image &, 
                               CandidateTree<SmoothedPixel>& result ) 
            const throw()
            { result.fill( *smoothed ); }

        const SmoothedImage& getSmoothed() const throw()
            { return *smoothed; }
   };

}

#endif
