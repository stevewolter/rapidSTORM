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
     const engine::BaseImage& image,
         int xl, int yl) 
{
    deriver.setData( 
        image.ptr(), 
        image.width() / cs_units::camera::pixel,
        image.height() / cs_units::camera::pixel );
    deriver.setUpperLeftCorner( xl, yl );

    Eigen::Matrix<double,1,1> corners =
        deriver.selectedData.template corner<1,1>(Eigen::TopLeft) +
        deriver.selectedData.template corner<1,1>(Eigen::TopRight) +
        deriver.selectedData.template corner<1,1>(Eigen::BottomRight) +
        deriver.selectedData.template corner<1,1>(Eigen::BottomLeft);
    double shift_estimate = corners.sum() / 4;
    
    common.set_start( spot, image, shift_estimate, &a.parameters );
     
    std::pair<fitpp::FitResult,typename Deriver::Position*>
        fitResult = 
            common.fit_function.fit(
                a, b, common.constants, deriver );

    c = fitResult.second;
    bool is_good
        = c != NULL &&
          common.check_result( &c->parameters, target );

    if ( is_good ) {
        return 1;
    } else
        return -1;
}

}
}

#endif
