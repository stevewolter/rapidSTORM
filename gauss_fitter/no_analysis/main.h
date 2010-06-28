#ifndef DSTORM_GAUSSFITTER_NOANALYSIS_H
#define DSTORM_GAUSSFITTER_NOANALYSIS_H

#include <fit++/Exponential2D.hh>
#include "fitter/FixedSized_decl.h"
#include "fitter/Sized.h"
#include "Config_decl.h"
#include <dStorm/engine/JobInfo_decl.h>
#include "fitter/MarquardtInfo.h"

namespace dStorm {
namespace gauss_2d_fitter {
namespace no_analysis {

template <int Kernels, int FitFlags>
class CommonInfo
: public fitter::MarquardtInfo
    <fitpp::Exponential2D::Model<Kernels, FitFlags>::VarC>
{
  protected:
    Eigen::Vector2i maxs;
    Eigen::Vector2d start;
    const double amplitude_threshold;
    const double start_sx, start_sy, start_sxy;

  public:
    typedef typename fitpp::Exponential2D::Model<Kernels, FitFlags>
        FitGroup;
    typename FitGroup::Constants constants;
    FitGroup params;

 public:
    typedef gauss_2d_fitter::Config Config;
    CommonInfo( const Config&, const engine::JobInfo& );
    CommonInfo( const CommonInfo& );
    void set_start(typename FitGroup::Variables* variables);
    void set_start( 
        const engine::Spot& spot, const engine::BaseImage& image,
        double shift_estimate, typename FitGroup::Variables* variables );
    bool check_result( typename FitGroup::Variables *variables, 
                       Localization *target);
};

template <int FitFlags, bool HonorCorrelation, int Kernels = 1>
struct Fitter {
    typedef CommonInfo<Kernels,FitFlags> SizeInvariants;
    template <int X, int Y> struct Specialized;
};

template <int FitFlags, bool HonorCorrelation, int Kernels>
template <int X, int Y> 
struct Fitter<FitFlags,HonorCorrelation,Kernels>::Specialized {
    typedef fitter::FixedSized<Fitter,X,Y> 
        Sized;
    typedef typename fitpp::Exponential2D::Model<Kernels, FitFlags>
        ::template Fitter<engine::StormPixel,X,Y,HonorCorrelation>::Type
        Deriver;
};

}
}
}

#endif
