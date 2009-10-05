#ifndef DSTORM_SPOTFINDER_H
#define DSTORM_SPOTFINDER_H

#include <dStorm/Image.h>
#include <dStorm/Spot.h>
#include <dStorm/CandidateTree.h>
#include <memory>

namespace dStorm {
   class Config;

   /** The SpotFinder class is the base class for all spot 
    *  finding mechanisms. */
   class SpotFinder {
      protected:
        SpotFinder(const Config &conf, int imw, int imh);
        const int msx, /**< Smoothing mask radius in X */
                  msy, /**< Smoothing mask radius in Y */
                  bx,  /**< Border (non-smoothed at image border)
                            radius in X */
                  by,  /**< Border (non-smoothed at image border)
                            radius in Y */
                  imw, /**< Width of the smoothed image */
                  imh; /**< Height of the smoothed image */
        /** Buffer that contains the smoothed image. */
        std::auto_ptr<SmoothedImage> smoothed; 

      public:
        enum SpotFinders { Average, Median, Erosion, Reconstruction,
                           Gaussian };
        virtual ~SpotFinder();

        /** Construct the spot finder given in the configuration
         *  \c conf and build an instance of it for an image
         *  \c imw wide and \c imh high. */
        static std::auto_ptr<SpotFinder> factory
            (const Config &conf, int imw, int imh);

        virtual void smooth( const Image &image ) = 0;

        const SmoothedImage& getSmoothedImage() const
            { return *smoothed; }
   };

}

#endif
