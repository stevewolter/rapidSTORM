#ifndef DSTORM_GAUSSFITTER_H
#define DSTORM_GAUSSFITTER_H

#include <fit++/Exponential2D.hh>

#include "SpotFitter.h"
#include "Config.h"

namespace dStorm {
   class GaussFitter : public SpotFitter {
      private:
         libfitpp::Exponential2D_FixedForm<StormPixel> fitter;
         const Config &config;
         int msx, msy;

         const Image *image;
         double amplitude_threshold;

         GaussFitter(const GaussFitter &) ;
          
      public:
         GaussFitter(const Config &config) throw();
         virtual ~GaussFitter() throw();
         void prepareToFitImage(const Image &im);

         bool fitSpot( const Spot& spot, Localization* target ) throw();

         EIGEN_MAKE_ALIGNED_OPERATOR_NEW
   };
};

#endif
