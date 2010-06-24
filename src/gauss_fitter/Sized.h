#ifndef GAUSS_FITTER_SPECIALIZED_H
#define GAUSS_FITTER_SPECIALIZED_H

#include "GaussFitter.h"
#include <dStorm/BaseImage.h>
#include <fit++/FitFunction_impl.hh>
#include <fitter/FixedSized_impl.h>

using namespace fitpp::Exponential2D;

namespace Eigen {
    template <>
    class NumTraits<unsigned short>
        : public NumTraits<int> {};
}

namespace dStorm {
namespace engine {

template <class BaseFitter, int Width, int Height>
class ResidueAnalyzingSized
: public fitter::FixedSized<BaseFitter,Width,Height>
{
    typedef typename BaseFitter::NoAnalysis::
        SpecializationInfo<Width,Height>::Sized Normal;
    typedef fitter::FixedSized<BaseFitter,Width,Height> Base;
    typedef fitter::FixedSized<,Width,Height
    typedef typename Base::Common Common;

    Normal normal;

  public:
    ResidueAnalyzingSized(Common& common) 
        : Base(common), common(common) {}

    void setSize( int width, int height ) {
        Base::setSize( width, height );
        normal.setSize( width, height );
    }

    int fit(const Spot& spot, Localization* target,
        const BaseImage &image, int xl, int yl );

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  private:
    enum SpotState { Single, Fishy, Double };
    SpotState residue_analysis(Eigen::Vector2i* direction,
                               int xl, int yl);
    float double_fit_analysis(
        const BaseImage& image, const Eigen::Vector2i& direction,
        int single_fit_xl, int single_fit_yl);
};

template <class BaseFitter, int Width, int Height>
int 
SpecializedGaussFitter<BaseFitter, Width, Height>::
fit( const Spot &spot, Localization *target, const BaseImage& image,
         int xl, int yl) 
{
    int one_fit = normal.fit( spot, target, image, xl, yl );
    if ( one_fit <= 0 )
        return one_fit;

    Eigen::Vector2i suspected_doubleSpot_direction;
    switch ( residue_analysis( &suspected_doubleSpot_direction, xl, yl ) ) 
    {
        case Single:
            target->two_kernel_improvement() = 0;
        case Double:
        case Fishy:
            target->two_kernel_improvement() = 1 - double_fit_analysis
                (image, suspected_doubleSpot_direction, xl, yl); 
    }
    return one_fit;
}

}
}

#endif
