#ifndef DSTORM_SPOTFITTER_H
#define DSTORM_SPOTFITTER_H

#include "Image_decl.h"
#include "FitPosition.h"
#include "../Localization_decl.h"
#include <memory>
#include <iterator>
#include <any_iterator.hpp>

namespace dStorm {
namespace engine {
namespace spot_fitter {

   /** Base class for spot fitting classes. Users should call
    *  prepareToFitImage() for each new image that should be fitted,
    *  and then fitSpot() for each position in that image that should
    *  be fitted. */
   class Implementation {
      public:
         virtual ~Implementation() {}

         typedef IteratorTypeErasure::any_iterator< dStorm::Localization, boost::incrementable_traversal_tag >
            iterator;

         /** \param spot        Position at which to fit
          *  \param im          Image to be fitted.
          *  \param number      Number of the image to be fitted.
          *  \return            Number of found spots */
         virtual int fitSpot( const FitPosition& spot, const ImageStack &im,
                              iterator target ) = 0;
   };

}
}
}

#endif
