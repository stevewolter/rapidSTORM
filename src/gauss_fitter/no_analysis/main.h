#ifndef DSTORM_GAUSSFITTER_NOANALYSIS_H
#define DSTORM_GAUSSFITTER_NOANALYSIS_H

#include <fit++/Exponential2D.hh>
#include "fitter/FixedSized_decl.h"
#include "fitter/Sized.h"
#include "Config_decl.h"
#include <dStorm/engine/JobInfo_decl.h>

namespace dStorm {
namespace gauss_2d_fitter {
namespace no_analysis {

template <int FitFlags>
class CommonInfo
{
  protected:
    Eigen::Vector2i maxs;
    Eigen::Vector2d start;
    typedef typename fitpp::Exponential2D::For<1, FitFlags> FitGroup;
    const double amplitude_threshold;
    const double start_sx, start_sy, start_sxy;

  public:
    typename FitGroup::Constants constants;
    fitpp::FitFunction<FitGroup::VarC> fit_function;
    typename FitGroup::NamedParameters params;

 public:
    CommonInfo( const Config&, const engine::JobInfo& );
    void set_start( 
        const engine::Spot& spot, const engine::BaseImage& image,
        double shift_estimate, typename FitGroup::Variables* variables );
    bool check_result( typename FitGroup::Variables *variables, 
                       Localization *target);
};

template <int FitFlags, bool HonorCorrelation>
struct Fitter {
    typedef CommonInfo<FitFlags> SizeInvariants;
    template <int X, int Y> struct Specialized;
};

template <int FitFlags, bool HonorCorrelation>
template <int X, int Y> 
struct Fitter<FitFlags,HonorCorrelation>::Specialized {
    typedef fitter::FixedSized<Fitter,X,Y> 
        Sized;
    typedef typename fitpp::Exponential2D::For<1, FitFlags>
        ::template Deriver<engine::StormPixel,X,Y,HonorCorrelation> 
        Deriver;
};

}
}
}

#endif
