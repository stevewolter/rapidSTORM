#ifndef GAUSS_FITTER_RESIDUE_ANALYSIS_H
#define GAUSS_FITTER_RESIDUE_ANALYSIS_H

#include "main.h"
#include <dStorm/BaseImage.h>
#include <fit++/FitFunction_impl.hh>
#include "fitter/FixedSized.h"

namespace Eigen {
    template <>
    class NumTraits<unsigned short>
        : public NumTraits<int> {};
}

namespace dStorm {

#if 0
namespace fitter {
/** Partial specialization of generic fitter fit() function to avoid
 *  compilation errors in unused generic fit() function. */
template <int FitFlags, bool HonorCorrelation, int Width, int Height>
int FixedSized< 
    gauss_2d_fitter::residue_analysis::Fitter<FitFlags,HonorCorrelation>,
    Width, Height >::fit(
        const engine::Spot&, Localization* ,
        const engine::BaseImage &, int, int ) 
    { assert( false ); return 0; }

}
#endif

namespace fitter {
namespace residue_analysis {

template <class BaseFitter, int Width, int Height>
class SizedFitter
: public BaseFitter::TwoKernel::template Specialized
    <Width,Height>::Sized
{
    typedef typename BaseFitter::OneKernel
        ::template Specialized <Width,Height>::Sized
        Normal;
    typedef typename BaseFitter::TwoKernel
        ::template Specialized <Width,Height>::Sized
        Base;
    typedef CommonInfo<typename BaseFitter::SizeInvariants> Common;

    Normal normal;
    CommonInfo<typename BaseFitter::SizeInvariants>& common;

  public:
    SizedFitter(Common& common) 
        : Base(common), normal(common), common(common) {}

    void setSize( int width, int height ) {
        Base::setSize( width, height );
        normal.setSize( width, height );
    }

    int fit(const engine::Spot& spot, Localization* target,
        const engine::BaseImage &image, int xl, int yl );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  private:
    enum SpotState { Single, Fishy, Double };
    SpotState residue_analysis(Eigen::Vector2i* direction,
                               int xl, int yl);
    float double_fit_analysis(
        const engine::BaseImage& image, 
        const Eigen::Vector2i& direction,
        int single_fit_xl, int single_fit_yl);
};

template <class BaseFitter, int Width, int Height>
int 
SizedFitter<BaseFitter, Width, Height>::
fit( const engine::Spot &spot, Localization *target,
     const engine::BaseImage& image, int xl, int yl) 
{
    int one_fit = normal.fit( spot, target, image, xl, yl );
    if ( one_fit <= 0 )
        return one_fit;

    Eigen::Vector2i suspected_doubleSpot_direction;
    float two_kernel_improvement = 0;
    if ( this->common.asymmetry_threshold <= 1E-3 ||
         residue_analysis( &suspected_doubleSpot_direction, xl, yl ) != Single ) 
    {
        two_kernel_improvement = 1 - double_fit_analysis
            (image, suspected_doubleSpot_direction, xl, yl); 
    }
    common.set_two_kernel_improvement( *target, two_kernel_improvement );
    return one_fit;
}

}
}
}

#include "analysis.h"

#endif
