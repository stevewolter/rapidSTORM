#ifndef DSTORM_GAUSSFITTER_RESIDUEANALYSIS_H
#define DSTORM_GAUSSFITTER_RESIDUEANALYSIS_H

#include "no_analysis/main.h"
#include <fit++/Exponential2D.hh>

namespace dStorm {
namespace gauss_2d_fitter {
namespace residue_analysis {

template <int FitFlags>
class CommonInfo
: public no_analysis::CommonInfo<2,FitFlags>,
  public no_analysis::CommonInfo<1,FitFlags>
{
    typedef no_analysis::CommonInfo<1,FitFlags> Base1;
    typedef no_analysis::CommonInfo<2,FitFlags> Base2;
  public:
    typedef typename Base1::FitGroup::Variables SingleFit;
    typedef typename Base2::FitGroup::Variables DoubleFit;

    typedef gauss_2d_fitter::Config Config;

    CommonInfo( const Config&, const engine::JobInfo& );

    void start_from_splitted_single_fit
        ( SingleFit& from, DoubleFit* v, 
          const Eigen::Vector2i& dir);
    float sq_peak_distance(DoubleFit *variables);
    void get_center(const SingleFit& position, int& x, int& y);
};

template <int FitFlags, bool HonorCorrelation>
struct Fitter {
    typedef CommonInfo<FitFlags> SizeInvariants;
    typedef no_analysis::Fitter<FitFlags,HonorCorrelation,1> OneKernel;
    typedef no_analysis::Fitter<FitFlags,HonorCorrelation,2> TwoKernel;
};

}
}
}

#endif
