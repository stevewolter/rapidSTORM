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
class FunctionParameters
: public fitpp::Exponential2D::Model<Kernels, FitFlags>
{
  public:
    typedef typename fitpp::Exponential2D::Model<Kernels, FitFlags>
        FitGroup;
    typedef typename FitGroup::Constants Constants;
    typedef typename FitGroup::Variables Variables;

    Constants constants;

    FunctionParameters() : FitGroup( NULL, &constants ) {}
    FunctionParameters(const FunctionParameters& o) 
        : FitGroup( NULL, &constants ), constants(o.constants) {}
};

template <int Kernels, int FitFlags>
class CommonInfo
: public fitter::MarquardtInfo
    <fitpp::Exponential2D::Model<Kernels, FitFlags>::VarC>
{
  protected:
    typedef typename FunctionParameters<Kernels,FitFlags>::Variables Variables;
    typedef typename FunctionParameters<Kernels,FitFlags>::FitGroup FitGroup;

    FunctionParameters<Kernels,FitFlags> params;
    Eigen::Vector2i maxs;
    Eigen::Vector2d start, scale_factor;
    const double amplitude_threshold;
    const double start_sx, start_sy, start_sxy;

    bool compute_uncertainty;
    const double photon_response_factor;
    double background_noise_variance;

  public:
    typedef gauss_2d_fitter::Config Config;
    CommonInfo( const Config&, const engine::JobInfo& );

    void set_start(Variables* variables);
    void set_start( 
        const engine::Spot& spot, const engine::BaseImage& image,
        double shift_estimate, Variables* variables );
    bool check_result( Variables *variables, Localization *target);

    typename FitGroup::Constants& constants() { return params.constants; }
    const typename FitGroup::Constants& constants() const { return params.constants; }
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
