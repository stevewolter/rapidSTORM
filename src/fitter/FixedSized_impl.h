#ifndef DSTORM_FITTER_CONCRETE_SIZE_IMPL_H
#define DSTORM_FITTER_CONCRETE_SIZE_IMPL_H

#include "FixedSized.h"

namespace dStorm {
namespace fitter {

template <class BaseFitter, int Width, int Height>
int 
FixedSized<BaseFitter, Width, Height>::
fit( const Spot &spot, Localization *target, const BaseImage& image,
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
    
    StartInformation starts =
        common.set_start( spot, image, shift_estimate, &a.parameters );
     
    std::pair<FitResult,typename Deriver::Position*>
        fitResult = 
            common.Width_Invariants<FitFlags,false>::fit_function.fit(
                a, b,
                common.Width_Invariants<FitFlags,false>::constants,
                deriver );

    c = fitResult.second;
    bool is_good
        = c != NULL &&
          common.check_result( &c->parameters, target, starts );

    if ( is_good ) {
        target->unset_source_trace();
        return 1;
    } else
        return -1;
}

}
}

#endif
