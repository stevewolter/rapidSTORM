#ifndef DSTORM_FITTER_CONCRETE_SIZE_IMPL_H
#define DSTORM_FITTER_CONCRETE_SIZE_IMPL_H

#include "FixedSized.h"
#include <dStorm/engine/Image.h>
#include "fit++/FitFunction.hh"

namespace dStorm {
namespace fitter {

template <class BaseFitter, int Width, int Height>
int 
FixedSized<BaseFitter, Width, Height>::
fit( const engine::Spot &spot,
     Localization *target, 
     const engine::Image& image,
         int xl, int yl) 
{
    deriver.setData( image );
    deriver.setUpperLeftCorner( xl, yl );

    double corner_sum = 0;
    for (int i = 0; i < Deriver::Depth; ++i) 
        corner_sum += 
            deriver.selectedData[i].template corner<1,1>(Eigen::TopLeft).sum() +
            deriver.selectedData[i].template corner<1,1>(Eigen::TopRight).sum() +
            deriver.selectedData[i].template corner<1,1>(Eigen::BottomRight).sum() +
            deriver.selectedData[i].template corner<1,1>(Eigen::BottomLeft).sum();
    double shift_estimate = corner_sum / (4 * Deriver::Depth);
    
    common.set_start( spot, image, shift_estimate, 
                      &deriver.getVariables() );
     
    fitpp::FitResult fitResult = 
        deriver.fit( common.fit_function );

    bool is_good = 
          (fitResult != fitpp::InvalidStartPosition) &&
          common.check_result( &deriver.getVariables(), deriver.getPosition().chi_sq, target );

    if ( is_good ) {
        return 1;
    } else
        return -1;
}

}
}

#endif
